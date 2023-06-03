#include "Metadata.hpp"
#include "ErrorMessage.hpp"
#include "ExceptionFFI.hpp"

#include "../clp/components/core/src/ffi/ir_stream/protocol_constants.hpp"

static inline bool is_valid_json_string_data (nlohmann::json const& data, char const* key) {
    return data.contains(key) && data[key].is_string();
};

namespace clp_ffi_py::metadata {
Metadata::Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding) {
    if (false == is_four_byte_encoding) {
        throw ExceptionFFI(ErrorCode_Unsupported,
                           __FILE__,
                           __LINE__,
                           "Eight Byte Preamble is not yet supported.");
    }
    m_is_four_byte_encoding = is_four_byte_encoding;

    // Read ref_timestamp
    auto const ref_timestamp_key{ffi::ir_stream::cProtocol::Metadata::ReferenceTimestampKey};
    if (false == is_valid_json_string_data(metadata, ref_timestamp_key)) {
        throw ExceptionFFI(ErrorCode_MetadataCorrupted,
                           __FILE__,
                           __LINE__,
                           "Reference Timestamp cannot be found in the metadata.");
    }

    try {
        const std::string ref_timestamp_str{metadata[ref_timestamp_key]};
        m_ref_timestamp = std::stoi(ref_timestamp_str);
    } catch (std::exception const& ex) {
        // Handle the exception triggered by stoi
        throw ExceptionFFI(ErrorCode_Unsupported, __FILE__, __LINE__, ex.what());
    }

    // Read timestamp format
    auto const timestamp_format_key{ffi::ir_stream::cProtocol::Metadata::TimestampPatternKey};
    if (false == is_valid_json_string_data(metadata, timestamp_format_key)) {
        throw ExceptionFFI(ErrorCode_MetadataCorrupted,
                           __FILE__,
                           __LINE__,
                           "Timestamp Format cannot be found in the metadata.");
    }
    m_timestamp_format = metadata[timestamp_format_key];

    // Read timezone
    auto const timezone_key{ffi::ir_stream::cProtocol::Metadata::TimeZoneIdKey};
    if (false == is_valid_json_string_data(metadata, timezone_key)) {
        throw ExceptionFFI(ErrorCode_MetadataCorrupted,
                           __FILE__,
                           __LINE__,
                           "TimezoneID cannot be found in the metadata.");
    }
    m_timezone = metadata[timezone_key];
}

PyObject* PyMetadata_new (PyTypeObject* type, PyObject* args, PyObject* kwds) {
    // Since tp_alloc returns <PyObject*>, we cannot use static_cast to cast it
    // to <PyMetadata*>. A C-style casting is expected (reinterpret_cast).
    PyMetadata* self{reinterpret_cast<PyMetadata*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->metadata = nullptr;
    return reinterpret_cast<PyObject*>(self);
}

int PyMetadata_init (PyMetadata* self, PyObject* args, PyObject* kwds) {
    ffi::epoch_time_ms_t ref_timestamp;
    char const* input_timestamp_format;
    char const* input_timezone;

    if (false ==
        PyArg_ParseTuple(args, "Lss", &ref_timestamp, &input_timestamp_format, &input_timezone)) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::arg_parsing_error);
        return -1;
    }

    self->metadata = new Metadata(ref_timestamp, input_timestamp_format, input_timezone);
    if (nullptr == self->metadata) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
        return -1;
    }

    return 0;
}

void PyMetadata_dealloc (PyMetadata* self) {
    delete self->metadata;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* PyMetadata_is_using_four_byte_encoding (PyMetadata* self) {
    assert(self->metadata);
    if (self->metadata->is_using_four_byte_encoding()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

PyObject* PyMetadata_get_ref_timestamp (PyMetadata* self) {
    assert(self->metadata);
    return PyLong_FromLongLong(self->metadata->get_ref_timestamp());
}

PyObject* PyMetadata_get_timestamp_format (PyMetadata* self) {
    assert(self->metadata);
    return PyUnicode_FromString(self->metadata->get_timestamp_format().c_str());
}

PyObject* PyMetadata_get_timezone (PyMetadata* self) {
    assert(self->metadata);
    return PyUnicode_FromString(self->metadata->get_timezone().c_str());
}

PyMetadata* PyMetadata_init_from_json (nlohmann::json const& metadata, bool is_four_byte_encoding) {
    PyMetadata* self{reinterpret_cast<PyMetadata*>(PyObject_New(
            PyMetadata, reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMetadataTy))))};
    if (nullptr == self) {
        return nullptr;
    }
    self->metadata = new Metadata(metadata, is_four_byte_encoding);
    if (nullptr == self->metadata) {
        Py_DECREF(self);
        return nullptr;
    }
    return self;
}

static PyMethodDef PyMetadata_method_table[] = {
        {"is_using_four_byte_encoding",
         reinterpret_cast<PyCFunction>(PyMetadata_is_using_four_byte_encoding),
         METH_NOARGS,
         "Check the encoding method (either 4byte or 8byte) from the metadata."},

        {"get_ref_timestamp",
         reinterpret_cast<PyCFunction>(PyMetadata_get_ref_timestamp),
         METH_NOARGS,
         "Get reference timestamp as an integer."},

        {"get_timestamp_format",
         reinterpret_cast<PyCFunction>(PyMetadata_get_timestamp_format),
         METH_NOARGS,
         "Get timestamp format as a string."},

        {"get_timezone",
         reinterpret_cast<PyCFunction>(PyMetadata_get_timezone),
         METH_NOARGS,
         "Get timezone as a string."},

        {nullptr}};

static PyType_Slot PyMetadata_slots[] = {
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMetadata_dealloc)},
        {Py_tp_methods, PyMetadata_method_table},
        {Py_tp_init, reinterpret_cast<void*>(PyMetadata_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyMetadata_new)},
        {0, nullptr}};

PyType_Spec PyMetadataTy = {
        "IRComponents.Metadata", sizeof(PyMetadata), 0, Py_TPFLAGS_DEFAULT, PyMetadata_slots};
} // namespace clp_ffi_py::metadata