#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyDecoderBuffer.hpp>

#include <iostream>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
void PyDecoderBuffer::shift() {
    auto const num_unread_bytes{buf_size - cursor_pos};

    if (num_unread_bytes > (buf_capacity / 2)) {
        // Grow the buffer if more than half of the bytes are unread
        this->grow_and_shift();
    } else {
        memcpy(buf, buf + cursor_pos, num_unread_bytes);
        // Reset cursor pos and buf size
        cursor_pos = 0;
        buf_size = num_unread_bytes;
    }
}

void PyDecoderBuffer::grow_and_shift() {
    assert(buf);
    auto const num_unread_bytes{buf_size - cursor_pos};
    auto const new_capacity{buf_capacity * 2};
    auto new_buf{reinterpret_cast<int8_t*>(PyMem_Malloc(new_capacity))};
    if (nullptr == new_buf) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
    }

    memcpy(new_buf, buf + cursor_pos, num_unread_bytes);

    // Clean old buffer
    PyMem_Free(buf);
    buf = new_buf;
    buf_capacity = new_capacity;

    // Reset cursor pos and buf size
    cursor_pos = 0;
    buf_size = num_unread_bytes;
}

auto PyDecoderBuffer::read_from(PyObject* istream) -> Py_ssize_t {
    this->shift();
    PyObject* retval =
            PyObject_CallMethod(istream, "readinto", "O", reinterpret_cast<PyObject*>(this));
    if (nullptr == retval) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::return_error);
        return -1;
    }
    Py_ssize_t num_bytes_read{PyLong_AsSsize_t(retval)};
    Py_DECREF(retval);

    buf_size += num_bytes_read;
    assert(buf_size <= buf_capacity);

    return num_bytes_read;
}

extern "C" {
static auto PyDecoderBuffer_new(PyTypeObject* type, PyObject* args, PyObject* keywords)
        -> PyObject* {
    PyDecoderBuffer* self{reinterpret_cast<PyDecoderBuffer*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }

    self->buf = reinterpret_cast<int8_t*>(PyMem_Malloc(initial_capacity));
    if (nullptr == self->buf) {
        Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->buf_capacity = initial_capacity;
    self->buf_size = 0;
    self->cursor_pos = 0;
    self->num_decoded_message = 0;

    return reinterpret_cast<PyObject*>(self);
}

static void PyDecoderBuffer_dealloc(PyDecoderBuffer* self) {
    PyMem_Free(self->buf);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static auto PyDecoderBuffer_read_from(PyDecoderBuffer* self, PyObject* args) -> PyObject* {
    PyObject* istream;
    if (!PyArg_ParseTuple(args, "O", &istream)) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::arg_parsing_error);
        Py_RETURN_NONE;
    }
    auto num_bytes_read{self->read_from(istream)};
    PyObject* py_integer{PyLong_FromSize_t(static_cast<size_t>(num_bytes_read))};
    return py_integer;
}

static auto PyDecoderBuffer_dump(PyDecoderBuffer* self) -> PyObject* {
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

static auto PyDecoderBuffer_getbuffer(PyDecoderBuffer* self, Py_buffer* view, int flags) -> int {
    assert(self->buf);
    auto const length{self->buf_capacity - self->buf_size};
    auto const data{self->buf + self->buf_size};
    return PyBuffer_FillInfo(view, reinterpret_cast<PyObject*>(self), data, length, 0, flags);
}

void PyDecoderBuffer_releasebuffer(PyDecoderBuffer* self, Py_buffer* view) {
    // Doesn't do anything
}
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

static PyBufferProcs PyDecoderBuffer_as_buffer{
        .bf_getbuffer = reinterpret_cast<getbufferproc>(PyDecoderBuffer_getbuffer),
        .bf_releasebuffer = reinterpret_cast<releasebufferproc>(PyDecoderBuffer_releasebuffer),
};

static PyType_Slot PyDecoderBuffer_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyDecoderBuffer_dealloc)},
        {Py_tp_methods, PyDecoderBuffer_method_table},
        {Py_tp_init, nullptr},
        {Py_tp_new, reinterpret_cast<void*>(PyDecoderBuffer_new)},
        {0, nullptr}};

static PyType_Spec PyDecoderBuffer_type_spec{
        "CLPIRDecoder.DecoderBuffer",
        sizeof(PyDecoderBuffer),
        0,
        Py_TPFLAGS_DEFAULT,
        PyDecoderBuffer_slots};

static std::unique_ptr<PyTypeObject, PyObjectDeleter<PyTypeObject>> PyDecoderBuffer_type;

auto PyDecoderBuffer_get_PyType() -> PyTypeObject* {
    return PyDecoderBuffer_type.get();
}

auto PyDecoderBuffer_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool {
    auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyDecoderBuffer_type_spec))};
    PyDecoderBuffer_type.reset(type);
    if (nullptr != type) {
        type->tp_as_buffer = &PyDecoderBuffer_as_buffer;
    }
    return add_type(
            reinterpret_cast<PyObject*>(PyDecoderBuffer_get_PyType()),
            "DecoderBuffer",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
