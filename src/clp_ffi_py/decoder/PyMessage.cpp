#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyMessage.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/decoder/Message.hpp>

namespace clp_ffi_py::decoder {
extern "C" {
static auto PyMessage_new(PyTypeObject* type, PyObject* args, PyObject* keywords) -> PyObject* {
    PyMessage* self{reinterpret_cast<PyMessage*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->message = new Message();
    if (nullptr == self->message) {
        Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    return reinterpret_cast<PyObject*>(self);
}

static void PyMessage_dealloc(PyMessage* self) {
    delete self->message;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static auto PyMessage_get_message(PyMessage* self) -> PyObject* {
    assert(self->message);
    return PyUnicode_FromString(self->message->get_message_ref().c_str());
}

static auto PyMessage_get_timestamp(PyMessage* self) -> PyObject* {
    assert(self->message);
    return PyLong_FromLongLong(self->message->get_timestamp_ref());
}

static auto PyMessage_get_message_idx(PyMessage* self) -> PyObject* {
    assert(self->message);
    return PyLong_FromLongLong(self->message->get_message_idx());
}

static auto PyMessage_wildcard_match(PyMessage* self, PyObject* args) -> PyObject* {
    char const* input_wildcard;
    Py_ssize_t input_wildcard_size;
    if (0 == PyArg_ParseTuple(args, "s#", &input_wildcard, &input_wildcard_size)) {
        return nullptr;
    }
    std::string_view wildcard{input_wildcard, static_cast<size_t>(input_wildcard_size)};
    if (self->message->wildcard_match(wildcard)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static auto PyMessage_wildcard_match_case_sensitive(PyMessage* self, PyObject* args) -> PyObject* {
    char const* input_wildcard;
    Py_ssize_t input_wildcard_size;
    if (0 == PyArg_ParseTuple(args, "s#", &input_wildcard, &input_wildcard_size)) {
        return nullptr;
    }
    std::string_view wildcard{input_wildcard, static_cast<size_t>(input_wildcard_size)};
    if (self->message->wildcard_match_case_sensitive(wildcard)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
}

auto PyMessage_create_empty() -> PyMessage* {
    PyMessage* self{reinterpret_cast<PyMessage*>(PyObject_New(PyMessage, PyMessage_get_PyType()))};
    if (nullptr == self) {
        return nullptr;
    }
    self->message = new Message();
    if (nullptr == self->message) {
        Py_DECREF(self);
        return nullptr;
    }
    return self;
}

auto PyMessage_create_new(std::string message, ffi::epoch_time_ms_t timestamp, size_t message_idx)
        -> PyMessage* {
    PyMessage* self{reinterpret_cast<PyMessage*>(PyObject_New(PyMessage, PyMessage_get_PyType()))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }
    self->message = new Message(message, timestamp, message_idx);
    if (nullptr == self->message) {
        Py_DECREF(self);
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }
    return self;
}

static PyMethodDef PyMessage_method_table[]{
        {"get_message",
         reinterpret_cast<PyCFunction>(PyMessage_get_message),
         METH_NOARGS,
         "Get message as a string."},
        {"get_timestamp",
         reinterpret_cast<PyCFunction>(PyMessage_get_timestamp),
         METH_NOARGS,
         "Get timestamp as a integer."},
        {"get_message_idx",
         reinterpret_cast<PyCFunction>(PyMessage_get_message_idx),
         METH_NOARGS,
         "Get message index as a integer."},
        {"wildcard_match",
         reinterpret_cast<PyCFunction>(PyMessage_wildcard_match),
         METH_VARARGS,
         "Wildcard match (case insensitive)"},
        {"wildcard_match_case_sensitive",
         reinterpret_cast<PyCFunction>(PyMessage_wildcard_match_case_sensitive),
         METH_VARARGS,
         "Wildcard match (case sensitive)"},
        {nullptr}};

static PyType_Slot PyMessage_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMessage_dealloc)},
        {Py_tp_methods, PyMessage_method_table},
        {Py_tp_init, nullptr},
        {Py_tp_new, reinterpret_cast<void*>(PyMessage_new)},
        {0, nullptr}};

static PyType_Spec PyMessage_type_spec{
        "CLPIRDecoder.Message",
        sizeof(PyMessage),
        0,
        Py_TPFLAGS_DEFAULT,
        PyMessage_slots};

auto PyMessage_get_PyType(bool init) -> PyTypeObject* {
    static std::unique_ptr<PyTypeObject, PyObjectDeleter<PyTypeObject>> PyMessage_type;
    if (init) {
        auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMessage_type_spec))};
        PyMessage_type.reset(type);
        if (nullptr != type) {
            Py_INCREF(type);
        }
    }
    return PyMessage_type.get();
}

auto PyMessageTy_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool {
    return add_type(
            reinterpret_cast<PyObject*>(PyMessage_get_PyType(true)),
            "Message",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
