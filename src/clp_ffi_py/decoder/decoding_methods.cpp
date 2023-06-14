#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/decoding_methods.hpp>

#include <clp/components/core/src/ffi/ir_stream/decoding_methods.hpp>
#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>
#include <string>

#include <clp_ffi_py/components/PyQuery.hpp>
#include <clp_ffi_py/decoder/PyDecoderBuffer.hpp>
#include <clp_ffi_py/ErrorMessage.hpp>

static inline Py_ssize_t
populate_buffer (clp_ffi_py::decoder::PyDecoderBuffer* buffer, PyObject* istream) {
    return buffer->read_from(istream);
}

namespace clp_ffi_py::decoder::four_byte_decoder {
clp_ffi_py::components::PyMessageCreateFuncType PyMessage_create_empty = nullptr;
clp_ffi_py::components::PyMetadataCreateFuncType PyMetadata_init_from_json = nullptr;

PyObject* decode_preamble (PyObject* self, PyObject* args) {
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
    auto metadata{PyMetadata_init_from_json(json_data, four_byte_encoding)};
    if (nullptr == metadata) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::decoder::invalid_metadata);
        return nullptr;
    }

    return reinterpret_cast<PyObject*>(metadata);
}

PyObject* decode_next_message (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t ref_timestamp;
    PyObject* istream{nullptr};
    PyObject* read_buffer_object{nullptr};

    if (false == PyArg_ParseTuple(args, "LOO", &ref_timestamp, &istream, &read_buffer_object)) {
        PyErr_SetString(PyExc_RuntimeError, error_messages::arg_parsing_error);
        return nullptr;
    }
    if (nullptr == istream || nullptr == read_buffer_object) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::arg_nullptr_error);
        return nullptr;
    }

    PyDecoderBuffer* read_buffer{reinterpret_cast<PyDecoderBuffer*>(read_buffer_object)};
    auto message{PyMessage_create_empty()};
    if (nullptr == message) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }
    ffi::epoch_time_ms_t timestamp_delta;

    while (true) {
        auto [buf_data, buf_size] = read_buffer->get_ir_buffer();
        ffi::ir_stream::IrBuffer ir_buffer{buf_data, buf_size};
        auto err{ffi::ir_stream::four_byte_encoding::decode_next_message(
                ir_buffer,
                message->message->get_message_ref(),
                timestamp_delta)};
        switch (err) {
        case ffi::ir_stream::IRErrorCode_Success:
            read_buffer->cursor_pos += ir_buffer.get_cursor_pos();
            message->message->set_timestamp(ref_timestamp + timestamp_delta);
            return reinterpret_cast<PyObject*>(message);
        case ffi::ir_stream::IRErrorCode_Incomplete_IR:
            if (auto num_bytes_read{read_buffer->read_from(istream)}; 0 == num_bytes_read) {
                PyErr_SetString(
                        PyExc_RuntimeError,
                        clp_ffi_py::error_messages::decoder::istream_empty_error);
                return nullptr;
            }
            break;
        case ffi::ir_stream::IRErrorCode_Eof:
            // Reaching the end of file, return None
            Py_DECREF(message);
            Py_RETURN_NONE;
        default:
            std::string error_message{
                    std::string(clp_ffi_py::error_messages::decoder::ir_error_code) +
                    std::to_string(err)};
            Py_DECREF(message);
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }
}

PyObject* decode_next_message_with_query (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t ref_timestamp;
    PyObject* istream{nullptr};
    PyObject* read_buffer_object{nullptr};
    PyObject* query_obj{nullptr};

    if (false ==
        PyArg_ParseTuple(args, "LOOO", &ref_timestamp, &istream, &read_buffer_object, &query_obj)) {
        PyErr_SetString(PyExc_RuntimeError, error_messages::arg_parsing_error);
        return nullptr;
    }
    if (nullptr == istream || nullptr == read_buffer_object || nullptr == query_obj) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::arg_nullptr_error);
        return nullptr;
    }

    PyDecoderBuffer* read_buffer{reinterpret_cast<PyDecoderBuffer*>(read_buffer_object)};
    auto query{reinterpret_cast<clp_ffi_py::components::PyQuery*>(query_obj)};
    auto message{PyMessage_create_empty()};
    if (nullptr == message) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }

    clp_ffi_py::components::Message decoded_message;
    while (true) {
        auto [buf_data, buf_size] = read_buffer->get_ir_buffer();
        ffi::ir_stream::IrBuffer ir_buffer{buf_data, buf_size};
        auto err{ffi::ir_stream::four_byte_encoding::decode_next_message(
                ir_buffer,
                decoded_message.get_message_ref(),
                decoded_message.get_timestamp_ref())};
        switch (err) {
        case ffi::ir_stream::IRErrorCode_Success:
            ref_timestamp += decoded_message.get_timestamp_ref();
            read_buffer->cursor_pos += ir_buffer.get_cursor_pos();
            if (false == query->query->matches(decoded_message)) {
                continue;
            }
            message->message->get_message_ref() = decoded_message.get_message_ref();
            message->message->get_timestamp_ref() = ref_timestamp;
            return reinterpret_cast<PyObject*>(message);
        case ffi::ir_stream::IRErrorCode_Incomplete_IR:
            if (auto num_bytes_read{read_buffer->read_from(istream)}; 0 == num_bytes_read) {
                PyErr_SetString(
                        PyExc_RuntimeError,
                        clp_ffi_py::error_messages::decoder::istream_empty_error);
                return nullptr;
            }
            break;
        case ffi::ir_stream::IRErrorCode_Eof:
            // Reaching the end of file, return None
            Py_DECREF(message);
            Py_RETURN_NONE;
        default:
            std::string error_message{
                    std::string(clp_ffi_py::error_messages::decoder::ir_error_code) +
                    std::to_string(err)};
            Py_DECREF(message);
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }
}
} // namespace clp_ffi_py::decoder::four_byte_decoder
