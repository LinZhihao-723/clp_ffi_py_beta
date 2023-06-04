#include "PyMessage.hpp"
#include "ErrorMessage.hpp"
#include "Message.hpp"

namespace clp_ffi_py::components {
PyObject* PyMessage_new (PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PyMessage* self{reinterpret_cast<PyMessage*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->message = new Message();
    if (nullptr == self->message) {
        Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
        Py_RETURN_NONE;
    }
    return reinterpret_cast<PyObject*>(self);
}

void PyMessage_dealloc (PyMessage* self) {
    delete self->message;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

PyObject* PyMessage_get_message (PyMessage* self) {
    assert(self->message);
    return PyUnicode_FromString(self->message->get_message_ref().c_str());
}

PyObject* PyMessage_get_timestamp (PyMessage* self) {
    assert(self->message);
    return PyLong_FromLongLong(self->message->get_timestamp_ref());
}

PyMessage* PyMessage_create () {
    PyMessage* self{reinterpret_cast<PyMessage*>(PyObject_New(
            PyMessage, reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMessageTy))))};
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

static PyMethodDef PyMessage_method_table[] = {
        {"get_message",
         reinterpret_cast<PyCFunction>(PyMessage_get_message),
         METH_NOARGS,
         "Get message as a string."},
        {"get_timestamp",
         reinterpret_cast<PyCFunction>(PyMessage_get_timestamp),
         METH_NOARGS,
         "Get timestamp as a integer."},
        {nullptr}};

static PyType_Slot PyMessage_slots[] = {{Py_tp_dealloc, reinterpret_cast<void*>(PyMessage_dealloc)},
                                        {Py_tp_methods, PyMessage_method_table},
                                        {Py_tp_init, nullptr},
                                        {Py_tp_new, reinterpret_cast<void*>(PyMessage_new)},
                                        {0, nullptr}};

PyType_Spec PyMessageTy = {
        "IRComponents.Message", sizeof(PyMessage), 0, Py_TPFLAGS_DEFAULT, PyMessage_slots};
} // namespace clp_ffi_py::components