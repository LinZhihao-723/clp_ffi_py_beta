#ifndef CLP_FFI_PY_PYDECODER_BUFFER
#define CLP_FFI_PY_PYDECODER_BUFFER

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <utility>

namespace clp_ffi_py::decoder {
constexpr const size_t initial_capacity{4096};

struct PyDecoderBuffer {
    PyObject_HEAD;
    int8_t* buf;
    Py_ssize_t cursor_pos;
    Py_ssize_t buf_size;
    Py_ssize_t buf_capacity;
    size_t num_decoded_message;

    [[nodiscard]] auto read_from(PyObject* istream) -> Py_ssize_t;
    [[nodiscard]] auto get_ir_buffer() const -> std::pair<int8_t*, size_t> {
        return {buf + cursor_pos, buf_size - cursor_pos};
    }

    auto get_num_decoded_message() const -> size_t { return num_decoded_message; }
    void increment_num_decoded_message() { ++num_decoded_message; }
    void increment_cursor(size_t offset) { cursor_pos += static_cast<Py_ssize_t>(offset); }

private:
    void grow_and_shift();
    void shift();
};

auto PyDecoderBuffer_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool;
auto PyDecoderBuffer_get_PyType() -> PyTypeObject*;
} // namespace clp_ffi_py::decoder
#endif
