#include <chrono>

#include "../clp/components/core/src/ffi/encoding_methods.hpp"
#include "../clp/components/core/src/ffi/ir_stream/encoding_methods.hpp"
#include "encoding_method.hpp"

namespace clp_ffi_py::encoder::four_byte_encoding {
char const* arg_parsing_error = "Failed to parse Python arguments.";
char const* timestamp_encoding_error = "Timestamp delta > signed int32 currently unsupported";
char const* preamble_encoding_error = "Metadata length > unsigned short currently unsupported";
char const* message_encoding_error = "Native encoder cannot handle the given message";

PyObject* encode_preamble (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t ref_timestamp;
    char const* input_timestamp_format;
    char const* input_timezone;

    if (false ==
        PyArg_ParseTuple(args, "Lss", &ref_timestamp, &input_timestamp_format, &input_timezone)) {
        PyErr_SetString(PyExc_RuntimeError, arg_parsing_error);
        Py_RETURN_NONE;
    }

    const std::string timestamp_format(input_timestamp_format);
    const std::string timezone(input_timezone);
    const std::string timestamp_pattern_syntax = "";
    std::vector<int8_t> ir_buf;

    if (false ==
        ffi::ir_stream::four_byte_encoding::encode_preamble(
                timestamp_format, timestamp_pattern_syntax, timezone, ref_timestamp, ir_buf)) {
        PyErr_SetString(PyExc_NotImplementedError, preamble_encoding_error);
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    // Release the input buffer
    return output_bytearray;
}

PyObject* encode_message_and_timestamp_delta (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t delta;
    Py_buffer input_buffer;
    if (false == PyArg_ParseTuple(args, "Ly*", &delta, &input_buffer)) {
        PyErr_SetString(PyExc_RuntimeError, arg_parsing_error);
        Py_RETURN_NONE;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;
    ir_buf.reserve(input_buffer.len * 2);
    std::string_view msg(static_cast<char const*>(input_buffer.buf), input_buffer.len);

    // Encode the message
    if (false == ffi::ir_stream::four_byte_encoding::encode_message(msg, logtype, ir_buf)) {
        PyBuffer_Release(&input_buffer);
        PyErr_SetString(PyExc_NotImplementedError, message_encoding_error);
        Py_RETURN_NONE;
    }

    // Encode the timestamp
    if (false == ffi::ir_stream::four_byte_encoding::encode_timestamp(delta, ir_buf)) {
        PyBuffer_Release(&input_buffer);
        PyErr_SetString(PyExc_NotImplementedError, timestamp_encoding_error);
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    PyBuffer_Release(&input_buffer);
    return output_bytearray;
}

PyObject* encode_message (PyObject* self, PyObject* args) {
    Py_buffer input_buffer;
    if (false == PyArg_ParseTuple(args, "y*", &input_buffer)) {
        PyErr_SetString(PyExc_RuntimeError, arg_parsing_error);
        Py_RETURN_NONE;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;
    ir_buf.reserve(input_buffer.len * 2);

    std::string_view msg(static_cast<char const*>(input_buffer.buf), input_buffer.len);
    if (false == ffi::ir_stream::four_byte_encoding::encode_message(msg, logtype, ir_buf)) {
        PyErr_SetString(PyExc_NotImplementedError, message_encoding_error);
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    PyBuffer_Release(&input_buffer);
    return output_bytearray;
}

PyObject* encode_timestamp_delta (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t delta;
    if (false == PyArg_ParseTuple(args, "L", &delta)) {
        PyErr_SetString(PyExc_RuntimeError, arg_parsing_error);
        Py_RETURN_NONE;
    }

    std::vector<int8_t> ir_buf;
    if (false == ffi::ir_stream::four_byte_encoding::encode_timestamp(delta, ir_buf)) {
        PyErr_SetString(PyExc_NotImplementedError, timestamp_encoding_error);
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    return output_bytearray;
}
} // namespace clp_ffi_py::encoder::four_byte_encoding