#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/decoding_methods.hpp>

#include <clp/components/core/src/ffi/ir_stream/decoding_methods.hpp>
#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>
#include <string>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/decoder/PyDecoderBuffer.hpp>
#include <clp_ffi_py/decoder/PyQuery.hpp>

static inline Py_ssize_t
populate_buffer(clp_ffi_py::decoder::PyDecoderBuffer* buffer, PyObject* istream) {
    return buffer->read_from(istream);
}

namespace clp_ffi_py::decoder::four_byte_decoder {
static auto
decode(ffi::epoch_time_ms_t ref_timestamp,
       PyObject* istream,
       PyDecoderBuffer* read_buffer,
       PyQuery* query,
       PyMetadata* metadata) -> PyObject* {
    std::string decoded_message;
    ffi::epoch_time_ms_t timestamp_delta;
    while (true) {
        auto [buf_data, buf_size] = read_buffer->get_ir_buffer();
        ffi::ir_stream::IrBuffer ir_buffer{buf_data, buf_size};
        auto err{ffi::ir_stream::four_byte_encoding::decode_next_message(
                ir_buffer,
                decoded_message,
                timestamp_delta)};
        switch (err) {
        case ffi::ir_stream::IRErrorCode_Success:
            ref_timestamp += timestamp_delta;
            read_buffer->increment_cursor(ir_buffer.get_cursor_pos());
            read_buffer->increment_num_decoded_message();
            if (nullptr != query) {
                // Since no one enforces the query lower bound to be smaller
                // than the upper bound, upper bound check should be executed
                // first to ensure early exit
                // Also, logs may not be sorted w.r.t. the timestamp, we cannot
                // early exit when the current ref timestamp is larger than the
                // upper bound unless it passes safe check
                if (false == query->query->ts_upper_bound_check(ref_timestamp)) {
                    if (query->query->ts_upper_bound_exit_check(ref_timestamp)) {
                        Py_RETURN_NONE;
                    }
                    continue;
                }
                if (false == query->query->ts_lower_bound_check(ref_timestamp)) {
                    continue;
                }
                if (false == query->query->matches(std::string_view(decoded_message))) {
                    continue;
                }
            }
            return reinterpret_cast<PyObject*>(PyMessage_create_new(
                    decoded_message,
                    ref_timestamp,
                    read_buffer->get_num_decoded_message() - 1,
                    metadata));
        case ffi::ir_stream::IRErrorCode_Incomplete_IR:
            if (auto num_bytes_read{read_buffer->read_from(istream)}; 0 == num_bytes_read) {
                // PyErr_SetString(
                //         PyExc_RuntimeError,
                //         clp_ffi_py::error_messages::decoder::istream_empty_error);
                // The stream is truncated . We should probably send a warning instead...
                Py_RETURN_NONE;
            }
            break;
        case ffi::ir_stream::IRErrorCode_Eof:
            // Reaching the end of file, return None
            Py_RETURN_NONE;
        default:
            std::string error_message{
                    std::string(clp_ffi_py::error_messages::decoder::ir_error_code) +
                    std::to_string(err)};
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }
}

extern "C" {
PyObject* decode_preamble(PyObject* self, PyObject* args) {
    PyObject* istream{nullptr};
    PyObject* read_buffer_object{nullptr};
    if (false == PyArg_ParseTuple(args, "OO", &istream, &read_buffer_object)) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::arg_parsing_error);
        return nullptr;
    }
    if (nullptr == istream || nullptr == read_buffer_object) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::arg_nullptr_error);
        return nullptr;
    }

    PyDecoderBuffer* read_buffer{reinterpret_cast<PyDecoderBuffer*>(read_buffer_object)};
    if (auto num_bytes_read{read_buffer->read_from(istream)}; 0 == num_bytes_read) {
        PyErr_SetString(
                PyExc_RuntimeError,
                clp_ffi_py::error_messages::decoder::istream_empty_error);
        return nullptr;
    }

    bool success;

    success = false;
    bool four_byte_encoding;
    while (false == success) {
        auto [buf_data, buf_size] = read_buffer->get_ir_buffer();
        ffi::ir_stream::IrBuffer ir_buffer{buf_data, buf_size};
        auto err{ffi::ir_stream::get_encoding_type(ir_buffer, four_byte_encoding)};
        switch (err) {
        case ffi::ir_stream::IRErrorCode_Success:
            read_buffer->cursor_pos += ir_buffer.get_cursor_pos();
            success = true;
            break;
        case ffi::ir_stream::IRErrorCode_Incomplete_IR:
            if (auto num_bytes_read{read_buffer->read_from(istream)}; 0 == num_bytes_read) {
                PyErr_SetString(
                        PyExc_RuntimeError,
                        clp_ffi_py::error_messages::decoder::istream_empty_error);
                return nullptr;
            }
            break;
        default:
            std::string error_message{
                    std::string(clp_ffi_py::error_messages::decoder::ir_error_code) +
                    std::to_string(err)};
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }

    if (false == four_byte_encoding) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::not_implemented_error);
        return nullptr;
    }

    ffi::ir_stream::encoded_tag_t metadata_type;
    size_t metadata_pos;
    uint16_t metadata_size;
    int8_t* metadata_start{nullptr};
    success = false;
    while (false == success) {
        auto [buf_data, buf_size] = read_buffer->get_ir_buffer();
        ffi::ir_stream::IrBuffer ir_buffer{buf_data, buf_size};
        auto err{ffi::ir_stream::decode_preamble(
                ir_buffer,
                metadata_type,
                metadata_pos,
                metadata_size)};
        switch (err) {
        case ffi::ir_stream::IRErrorCode_Success:
            metadata_start = buf_data + metadata_pos;
            read_buffer->cursor_pos += ir_buffer.get_cursor_pos();
            success = true;
            break;
        case ffi::ir_stream::IRErrorCode_Incomplete_IR:
            if (auto num_bytes_read{read_buffer->read_from(istream)}; 0 == num_bytes_read) {
                PyErr_SetString(
                        PyExc_RuntimeError,
                        clp_ffi_py::error_messages::decoder::istream_empty_error);
                return nullptr;
            }
            break;
        default:
            std::string error_message{
                    std::string(clp_ffi_py::error_messages::decoder::ir_error_code) +
                    std::to_string(err)};
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }

    assert(metadata_start);
    std::string json_string(reinterpret_cast<char*>(metadata_start), metadata_size);
    nlohmann::json json_data = nlohmann::json::parse(json_string);
    auto metadata{clp_ffi_py::decoder::PyMetadata_init_from_json(json_data, four_byte_encoding)};
    if (nullptr == metadata) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::decoder::invalid_metadata);
        return nullptr;
    }
    return reinterpret_cast<PyObject*>(metadata);
}

PyObject* decode_next_message(PyObject* self, PyObject* args, PyObject* keywords) {
    static char keyword_ref_timestamp[]{"ref_timestamp"};
    static char keyword_istream[]{"istream"};
    static char keyword_read_buffer[]{"read_buffer"};
    static char keyword_metadata[]{"metadata"};
    static char keyword_query[]{"query"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_ref_timestamp),
            static_cast<char*>(keyword_istream),
            static_cast<char*>(keyword_read_buffer),
            static_cast<char*>(keyword_metadata),
            static_cast<char*>(keyword_query),
            nullptr};

    ffi::epoch_time_ms_t ref_timestamp;
    PyObject* istream{nullptr};
    PyObject* read_buffer_obj{nullptr};
    PyObject* metadata_obj{nullptr};
    PyObject* query_obj{Py_None};

    if (false == PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "LOO!O!|O",
                         keyword_table,
                         &ref_timestamp,
                         &istream,
                         PyDecoderBuffer_get_PyType(),
                         &read_buffer_obj,
                         PyMetadata_get_PyType(),
                         &metadata_obj,
                         &query_obj)) {
        return nullptr;
    }

    bool is_query_given{Py_None != query_obj};
    if (is_query_given && false == PyObject_TypeCheck(query_obj, PyQuery_get_PyType())) {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
        return nullptr;
    }

    return decode(
            ref_timestamp,
            istream,
            reinterpret_cast<PyDecoderBuffer*>(read_buffer_obj),
            is_query_given ? reinterpret_cast<PyQuery*>(query_obj) : nullptr,
            reinterpret_cast<PyMetadata*>(metadata_obj));
}
}
} // namespace clp_ffi_py::decoder::four_byte_decoder
