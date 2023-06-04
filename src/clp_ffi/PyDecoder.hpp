#ifndef CLP_PY_PYDECODER
#define CLP_PY_PYDECODER

#include <Python.h>

namespace clp_ffi_py::decoder {
constexpr const size_t initial_capacity{4096};

struct PyBufferProxy {
    PyObject_HEAD;
    uint8_t* data;
    Py_ssize_t size;
};

struct PyDecoderBuffer {
    PyObject_HEAD;
    uint8_t* buf;
    Py_ssize_t cursor_pos;
    Py_ssize_t buf_size;
    Py_ssize_t buf_capacity;
    void grow ();

    Py_ssize_t read_into_buf (PyObject* istream);
    PyBufferProxy* proxy;
    PyBufferProxy* get_proxy ();
};

extern PyType_Spec PyDecoderBufferTy;

PyObject* PyDecoderBuffer_new (PyTypeObject* type, PyObject* args, PyObject* kwds);
void PyDecoderBuffer_dealloc (PyDecoderBuffer* self);
PyObject* PyDecoderBuffer_read_from (PyDecoderBuffer* self, PyObject* args);
PyObject* PyDecoderBuffer_dump (PyDecoderBuffer* self);
} // namespace clp_ffi_py::decoder
#endif