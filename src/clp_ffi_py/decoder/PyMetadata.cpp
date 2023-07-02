#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyMetadata.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/decoder/Metadata.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
static std::unique_ptr<PyTypeObject, PyObjectDeleter<PyTypeObject>> PyMetadata_type;

static auto PyMetadata_init_timezone(PyMetadata* self) -> bool {
    assert(self->metadata);
    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> func_args_ptr{
            Py_BuildValue("(s)", self->metadata->get_timezone_id().c_str())};
    auto func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return false;
    }
    self->Py_timezone = clp_ffi_py::Py_utils_get_timezone_from_timezone_id(func_args);
    if (nullptr == self->Py_timezone) {
        return false;
    }
    return true;
}

static auto PyMetadata_clear(PyMetadata* self) {
    self->metadata = nullptr;
    self->Py_timezone = Py_None;
}

extern "C" {
static auto PyMetadata_init(PyMetadata* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_ref_timestamp[]{"ref_timestamp"};
    static char keyword_timestamp_format[]{"timestamp_format"};
    static char keyword_timezone_id[]{"timezone_id"};
    static char* keyword_table[] = {
            static_cast<char*>(keyword_ref_timestamp),
            static_cast<char*>(keyword_timestamp_format),
            static_cast<char*>(keyword_timezone_id),
            nullptr};

    ffi::epoch_time_ms_t ref_timestamp;
    char const* input_timestamp_format;
    char const* input_timezone;

    if (false == PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "Lss",
                         keyword_table,
                         &ref_timestamp,
                         &input_timestamp_format,
                         &input_timezone)) {
        return -1;
    }

    PyMetadata_clear(self);
    self->metadata = new Metadata(ref_timestamp, input_timestamp_format, input_timezone);
    if (nullptr == self->metadata) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return -1;
    }
    if (false == PyMetadata_init_timezone(self)) {
        return -1;
    }

    return 0;
}

void PyMetadata_dealloc(PyMetadata* self) {
    delete self->metadata;
    Py_XDECREF(self->Py_timezone);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static auto PyMetadata_is_using_four_byte_encoding(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    if (self->metadata->is_using_four_byte_encoding()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static auto PyMetadata_get_ref_timestamp(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    return PyLong_FromLongLong(self->metadata->get_ref_timestamp());
}

static auto PyMetadata_get_timestamp_format(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    return PyUnicode_FromString(self->metadata->get_timestamp_format().c_str());
}

static auto PyMetadata_get_timezone_id(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    return PyUnicode_FromString(self->metadata->get_timezone_id().c_str());
}
}

auto PyMetadata_init_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding)
        -> PyMetadata* {
    PyMetadata* self{
            reinterpret_cast<PyMetadata*>(PyObject_New(PyMetadata, PyMetadata_get_PyType()))};
    if (nullptr == self) {
        return nullptr;
    }
    PyMetadata_clear(self);
    self->metadata = new Metadata(metadata, is_four_byte_encoding);
    if (nullptr == self->metadata) {
        Py_DECREF(self);
        return nullptr;
    }
    if (false == PyMetadata_init_timezone(self)) {
        Py_DECREF(self);
        return nullptr;
    }
    return self;
}

static PyMethodDef PyMetadata_method_table[]{
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

        {"get_timezone_id",
         reinterpret_cast<PyCFunction>(PyMetadata_get_timezone_id),
         METH_NOARGS,
         "Get timezone id as a string."},

        {nullptr}};

static PyMemberDef PyMetadata_members[] = {
        {"timezone",
         T_OBJECT,
         offsetof(PyMetadata, Py_timezone),
         READONLY,
         "Read only timezone stored as tzinfo"},
        {nullptr}};

static PyType_Slot PyMetadata_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMetadata_dealloc)},
        {Py_tp_methods, PyMetadata_method_table},
        {Py_tp_init, reinterpret_cast<void*>(PyMetadata_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_members, PyMetadata_members},
        {0, nullptr}};

static PyType_Spec PyMetadata_type_spec{
        "CLPIRDecoder.Metadata",
        sizeof(PyMetadata),
        0,
        Py_TPFLAGS_DEFAULT,
        PyMetadata_slots};

auto PyMetadata_get_PyType() -> PyTypeObject* {
    return PyMetadata_type.get();
}

auto PyMetadata_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool {
    auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMetadata_type_spec))};
    PyMetadata_type.reset(type);
    return add_type(
            reinterpret_cast<PyObject*>(PyMetadata_get_PyType()),
            "Metadata",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
