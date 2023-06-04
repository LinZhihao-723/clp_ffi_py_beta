#include "PyDecoder.hpp"
#include "ErrorMessage.hpp"

#include <iostream>

namespace clp_ffi_py::decoder {
PyObject* PyDecoderBuffer_new (PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PyDecoderBuffer* self{reinterpret_cast<PyDecoderBuffer*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
        Py_RETURN_NONE;
    }

    self->buf = reinterpret_cast<uint8_t*>(PyMem_Malloc(initial_capacity));
    if (nullptr == self->buf) {
        Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->buf_capacity = initial_capacity;
    self->buf_size = 0;
    self->cursor_pos = 0;

    return reinterpret_cast<PyObject*>(self);
}

void PyDecoderBuffer_dealloc (PyDecoderBuffer* self) {
    PyMem_Free(self->buf);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

void PyDecoderBuffer::grow() {
    assert(buf);
    auto const new_capacity{buf_capacity * 2};
    auto new_buf{reinterpret_cast<uint8_t*>(PyMem_Malloc(new_capacity * 2))};
    if (nullptr == new_buf) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::out_of_memory_error);
    }
    buf_capacity = new_capacity;
    auto const num_unread_bytes{buf_size - cursor_pos};
    if (0 != num_unread_bytes) {
        memcpy(new_buf, buf + cursor_pos, num_unread_bytes);
    }
    PyMem_Free(buf);
    buf = new_buf;

    // Reset cursor pos and buf size
    cursor_pos = 0;
    buf_size = num_unread_bytes;
}

Py_ssize_t PyDecoderBuffer::read_into_buf(PyObject* istream) {
    PyObject* retval =
            PyObject_CallMethod(istream, "readinto", "O", reinterpret_cast<PyObject*>(this));
    if (nullptr == retval) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::return_error);
        return -1;
    }
    Py_ssize_t num_bytes_read{PyLong_AsSsize_t(retval)};
    Py_DECREF(retval);

    buf_size += num_bytes_read;
    assert(buf_size <= buf_capacity);

    return num_bytes_read;
}

PyObject* PyDecoderBuffer_read_from (PyDecoderBuffer* self, PyObject* args) {
    PyObject* istream;
    if (!PyArg_ParseTuple(args, "O", &istream)) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::ErrorMessage::arg_parsing_error);
        Py_RETURN_NONE;
    }
    self->read_into_buf(istream);
    std::cerr << "Successfully Returned\n";
    Py_RETURN_NONE;
}

PyObject* PyDecoderBuffer_dump (PyDecoderBuffer* self) {
    assert(self);
    assert(self->buf);
    std::cerr << "Cursor position: " << self->cursor_pos << "\n";
    std::cerr << "Buffer size: " << self->buf_size << "\n";
    std::cerr << "Buffer capacity: " << self->buf_capacity << "\n";
    std::cerr << "Content: \n";
    for (Py_ssize_t i = 0; i < self->buf_size; ++i) {
        std::cerr << static_cast<char>(self->buf[i]);
    }
    std::cerr << "\n\n";
    Py_RETURN_NONE;
}

int PyDecoderBuffer_getbuffer (PyDecoderBuffer* self, Py_buffer* view, int flags) {
    // Implement your getbuffer logic here
    assert(self->buf);
    auto const length{self->buf_capacity - self->buf_size};
    auto const data{self->buf + self->buf_size};
    return PyBuffer_FillInfo(view, reinterpret_cast<PyObject*>(self), data, length, 0, flags);
}

void PyDecoderBuffer_releasebuffer (PyDecoderBuffer* self, Py_buffer* view) {
    // Doesn't do anything
}

static PyMethodDef PyDecoderBuffer_method_table[] = {
        {"read_from",
         reinterpret_cast<PyCFunction>(PyDecoderBuffer_read_from),
         METH_VARARGS,
         "Read from a stream and populate the buffer."},
        {"dump",
         reinterpret_cast<PyCFunction>(PyDecoderBuffer_dump),
         METH_NOARGS,
         "Dump the buffer."},
        {nullptr}};

static PyType_Slot PyDecoderBuffer_slots[] = {
        {Py_tp_dealloc, reinterpret_cast<void*>(PyDecoderBuffer_dealloc)},
        {Py_tp_methods, PyDecoderBuffer_method_table},
        {Py_tp_init, nullptr},
        {Py_bf_getbuffer, reinterpret_cast<void*>(PyDecoderBuffer_getbuffer)},
        {Py_bf_releasebuffer, reinterpret_cast<void*>(PyDecoderBuffer_releasebuffer)},
        {Py_tp_new, reinterpret_cast<void*>(PyDecoderBuffer_new)},
        {0, nullptr}};

PyType_Spec PyDecoderBufferTy = {"IRComponents.DecoderBuffer",
                                 sizeof(PyDecoderBuffer),
                                 0,
                                 Py_TPFLAGS_DEFAULT,
                                 PyDecoderBuffer_slots};
} // namespace clp_ffi_py::decoder
