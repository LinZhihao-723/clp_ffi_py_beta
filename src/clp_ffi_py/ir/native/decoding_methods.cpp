#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "decoding_methods.hpp"

#include <clp/components/core/src/BufferReader.hpp>
#include <clp/components/core/src/ffi/ir_stream/attributes.hpp>
#include <clp/components/core/src/ffi/ir_stream/decoding_methods.hpp>
#include <clp/components/core/src/ffi/ir_stream/encoding_methods.hpp>
#include <clp/components/core/src/type_utils.hpp>
#include <gsl/span>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/ir/native/PyDecoderBuffer.hpp>
#include <clp_ffi_py/ir/native/PyLogEvent.hpp>
#include <clp_ffi_py/ir/native/PyMetadata.hpp>
#include <clp_ffi_py/ir/native/PyQuery.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
/**
 * Decodes the next log event from the CLP IR buffer `decoder_buffer`. If
 * `py_query` is non-null decode until finding a log event that matches the
 * query.
 * @param decoder_buffer IR decoder buffer of the input IR stream.
 * @param py_metadata Theffi::ir_stream::IrBuffer metadata associated with the input IR stream.
 * @param py_query Search query to filter log events.
 * @param allow_incomplete_stream A flag to indicate whether the incomplete
 * stream error should be ignored. If it is set to true, incomplete stream error
 * should be treated as the termination.
 * @param cache_encoded_log_event A flag to indicate whether to cache the
 * encoded log event. The buffered log event will contain all the encoded
 * attributes, variables, and the logtype. The encoded timestamp delta is not
 * cached because it should be recalculated whenever to reuse the cached
 * encoded results.
 * @return Log event represented as PyLogEvent on success.
 * @return PyNone on termination.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto decode(
        PyDecoderBuffer* decoder_buffer,
        PyMetadata* py_metadata,
        PyQuery* py_query,
        bool allow_incomplete_stream,
        bool cache_encoded_log_event
) -> PyObject* {
    std::string decoded_message;
    ffi::epoch_time_ms_t timestamp_delta{0};
    auto timestamp{decoder_buffer->get_ref_timestamp()};
    auto const num_attributes{py_metadata->get_metadata()->get_num_attributes()};
    auto const& attribute_info_table{py_metadata->get_metadata()->get_attribute_table()};
    auto const& attribute_idx_map{py_metadata->get_metadata()->get_attribute_idx_map()};
    std::vector<std::optional<ffi::ir_stream::Attribute>> decoded_attributes;
    size_t current_log_event_idx{0};
    bool reached_eof{false};
    gsl::span<int8_t> encoded_log_event_view;

    while (true) {
        auto const unconsumed_bytes{decoder_buffer->get_unconsumed_bytes()};
        BufferReader ir_buffer{
                size_checked_pointer_cast<char const>(unconsumed_bytes.data()),
                unconsumed_bytes.size()
        };
        auto const err{ffi::ir_stream::four_byte_encoding::decode_next_message_with_attributes(
                ir_buffer,
                decoded_message,
                timestamp_delta,
                decoded_attributes,
                num_attributes
        )};
        if (ffi::ir_stream::IRErrorCode_Incomplete_IR == err) {
            if (false == decoder_buffer->try_read()) {
                if (allow_incomplete_stream
                    && static_cast<bool>(PyErr_ExceptionMatches(
                            PyDecoderBuffer::get_py_incomplete_stream_error()
                    )))
                {
                    PyErr_Clear();
                    Py_RETURN_NONE;
                }
                return nullptr;
            }
            continue;
        }
        if (ffi::ir_stream::IRErrorCode_Eof == err) {
            reached_eof = true;
            break;
        }
        if (ffi::ir_stream::IRErrorCode_Success != err) {
            PyErr_Format(PyExc_RuntimeError, cDecoderErrorCodeFormatStr, err);
            return nullptr;
        }

        if (false == ffi::ir_stream::validate_attributes(attribute_info_table, decoded_attributes))
        {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    "The decoded attributes do not match the declared ones in the metadata"
            );
            return nullptr;
        }
        timestamp += timestamp_delta;
        current_log_event_idx = decoder_buffer->get_and_increment_decoded_message_count();
        auto const curr_pos{ir_buffer.get_pos()};
        decoder_buffer->commit_read_buffer_consumption(curr_pos, encoded_log_event_view);

        if (nullptr == py_query) {
            break;
        }

        auto* query{py_query->get_query()};
        if (query->ts_safely_outside_time_range(timestamp)) {
            Py_RETURN_NONE;
        }
        bool matches{false};
        try {
            matches = query->matches_time_range(timestamp)
                      && query->matches_wildcard_queries(decoded_message)
                      && query->matches_decoded_attributes(decoded_attributes, attribute_idx_map);
        } catch (ExceptionFFI const& ex) {
            PyErr_Format(PyExc_RuntimeError, "Failed to match the queries: %s", ex.what());
            return nullptr;
        }
        if (matches) {
            break;
        }
    }

    if (reached_eof) {
        Py_RETURN_NONE;
    }

    decoder_buffer->set_ref_timestamp(timestamp);
    LogEvent::attribute_table_t attributes;
    for (size_t i{0}; i < attribute_info_table.size(); ++i) {
        attributes.emplace(attribute_info_table[i].get_name(), decoded_attributes[i]);
    }
    if (false == cache_encoded_log_event) {
        return py_reinterpret_cast<PyObject>(PyLogEvent::create_new_log_event(
                decoded_message,
                timestamp,
                current_log_event_idx,
                py_metadata,
                attributes
        ));
    }
    auto const encoded_timestamp_delta_size{
            ffi::ir_stream::four_byte_encoding::get_encoded_timestamp_delta_size(timestamp_delta)
    };
    auto const encoded_log_event_size_without_ts_delta{
            encoded_log_event_view.size() - encoded_timestamp_delta_size
    };
    return py_reinterpret_cast<PyObject>(PyLogEvent::create_new_log_event(
            decoded_message,
            timestamp,
            current_log_event_idx,
            py_metadata,
            attributes,
            encoded_log_event_view.subspan(0, encoded_log_event_size_without_ts_delta)
    ));
}
}  // namespace

extern "C" {
auto decode_preamble(PyObject* Py_UNUSED(self), PyObject* py_decoder_buffer) -> PyObject* {
    if (false
        == static_cast<bool>(PyObject_TypeCheck(py_decoder_buffer, PyDecoderBuffer::get_py_type())))
    {
        PyErr_SetString(PyExc_TypeError, cPyTypeError);
        return nullptr;
    }

    auto* decoder_buffer{py_reinterpret_cast<PyDecoderBuffer>(py_decoder_buffer)};
    bool is_four_byte_encoding{false};
    size_t ir_buffer_cursor_pos{0};
    while (true) {
        auto const unconsumed_bytes{decoder_buffer->get_unconsumed_bytes()};
        BufferReader ir_buffer{
                size_checked_pointer_cast<char const>(unconsumed_bytes.data()),
                unconsumed_bytes.size()
        };
        auto const err{ffi::ir_stream::get_encoding_type(ir_buffer, is_four_byte_encoding)};
        if (ffi::ir_stream::IRErrorCode_Success == err) {
            ir_buffer_cursor_pos = ir_buffer.get_pos();
            break;
        }
        if (ffi::ir_stream::IRErrorCode_Incomplete_IR != err) {
            PyErr_Format(PyExc_RuntimeError, cDecoderErrorCodeFormatStr, err);
            return nullptr;
        }
        if (false == decoder_buffer->try_read()) {
            return nullptr;
        }
    }
    decoder_buffer->commit_read_buffer_consumption(static_cast<Py_ssize_t>(ir_buffer_cursor_pos));
    if (false == is_four_byte_encoding) {
        PyErr_SetString(PyExc_NotImplementedError, "8-byte IR decoding is not supported yet.");
        return nullptr;
    }

    ffi::ir_stream::encoded_tag_t metadata_type_tag{0};
    size_t metadata_pos{0};
    uint16_t metadata_size{0};
    while (true) {
        auto const unconsumed_bytes = decoder_buffer->get_unconsumed_bytes();
        BufferReader ir_buffer{
                size_checked_pointer_cast<char const>(unconsumed_bytes.data()),
                unconsumed_bytes.size()
        };
        auto const err{ffi::ir_stream::decode_preamble(
                ir_buffer,
                metadata_type_tag,
                metadata_pos,
                metadata_size
        )};
        if (ffi::ir_stream::IRErrorCode_Success == err) {
            ir_buffer_cursor_pos = ir_buffer.get_pos();
            break;
        }
        if (ffi::ir_stream::IRErrorCode_Incomplete_IR != err) {
            PyErr_Format(PyExc_RuntimeError, cDecoderErrorCodeFormatStr, err);
            return nullptr;
        }
        if (false == decoder_buffer->try_read()) {
            return nullptr;
        }
    }

    auto const unconsumed_bytes = decoder_buffer->get_unconsumed_bytes();
    auto const metadata_buffer{
            unconsumed_bytes.subspan(metadata_pos, static_cast<size_t>(metadata_size))
    };
    decoder_buffer->commit_read_buffer_consumption(static_cast<Py_ssize_t>(ir_buffer_cursor_pos));
    PyMetadata* metadata{nullptr};
    try {
        // Initialization list should not be used in this case:
        // https://github.com/nlohmann/json/discussions/4096
        nlohmann::json const metadata_json(
                nlohmann::json::parse(metadata_buffer.begin(), metadata_buffer.end())
        );
        metadata = PyMetadata::create_new_from_json(metadata_json, is_four_byte_encoding);
    } catch (nlohmann::json::exception& ex) {
        PyErr_Format(PyExc_RuntimeError, "Json Parsing Error: %s", ex.what());
        return nullptr;
    }
    if (false == decoder_buffer->metadata_init(metadata)) {
        return nullptr;
    }
    return py_reinterpret_cast<PyObject>(metadata);
}

auto decode_next_log_event(PyObject* Py_UNUSED(self), PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_decoder_buffer[]{"decoder_buffer"};
    static char keyword_query[]{"query"};
    static char keyword_allow_incomplete_stream[]{"allow_incomplete_stream"};
    static char keyword_cache_encoded_log_event[]{"cache_encoded_log_event"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_decoder_buffer),
            static_cast<char*>(keyword_query),
            static_cast<char*>(keyword_allow_incomplete_stream),
            static_cast<char*>(keyword_cache_encoded_log_event),
            nullptr
    };

    PyDecoderBuffer* decoder_buffer{nullptr};
    PyObject* query{Py_None};
    int allow_incomplete_stream{0};
    int cache_encoded_log_event{0};

    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O!|Opp",
                static_cast<char**>(keyword_table),
                PyDecoderBuffer::get_py_type(),
                &decoder_buffer,
                &query,
                &allow_incomplete_stream,
                &cache_encoded_log_event
        )))
    {
        return nullptr;
    }

    bool const is_query_given{Py_None != query};
    if (is_query_given
        && false == static_cast<bool>(PyObject_TypeCheck(query, PyQuery::get_py_type())))
    {
        PyErr_SetString(PyExc_TypeError, cPyTypeError);
        return nullptr;
    }

    if (false == decoder_buffer->has_metadata()) {
        PyErr_SetString(
                PyExc_RuntimeError,
                "The given DecoderBuffer does not have a valid CLP IR metadata decoded."
        );
        return nullptr;
    }

    return decode(
            decoder_buffer,
            decoder_buffer->get_metadata(),
            is_query_given ? py_reinterpret_cast<PyQuery>(query) : nullptr,
            static_cast<bool>(allow_incomplete_stream),
            static_cast<bool>(cache_encoded_log_event)
    );
}
}
}  // namespace clp_ffi_py::ir::native
