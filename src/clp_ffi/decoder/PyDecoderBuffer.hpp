#ifndef CLP_FFI_PY_PYDECODER_BUFFER
#define CLP_FFI_PY_PYDECODER_BUFFER

#include "../Python.hpp"
#include <utility>

namespace clp_ffi_py::decoder {
constexpr const size_t initial_capacity{4096};

struct PyDecoderBuffer {
    PyObject_HEAD;
    int8_t* buf;
    Py_ssize_t cursor_pos;
    Py_ssize_t buf_size;
    Py_ssize_t buf_capacity;

    [[nodiscard]] Py_ssize_t read_from (PyObject* istream);
    [[nodiscard]] std::pair<int8_t*, size_t> get_ir_buffer () {
        return {buf + cursor_pos, buf_size - cursor_pos};
    }

private:
    void grow_and_shift ();
    void shift ();
};

PyObject* PyDecoderBuffer_get_PyType ();

PyObject* PyDecoderBuffer_new (PyTypeObject* type, PyObject* args, PyObject* kwds);
void PyDecoderBuffer_dealloc (PyDecoderBuffer* self);
PyObject* PyDecoderBuffer_read_from (PyDecoderBuffer* self, PyObject* args);
PyObject* PyDecoderBuffer_dump (PyDecoderBuffer* self);
} // namespace clp_ffi_py::decoder
#endif