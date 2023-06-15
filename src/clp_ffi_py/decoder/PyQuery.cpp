#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyQuery.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/decoder/PyMessage.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
auto PyQuery_new(PyTypeObject* type, PyObject* args, PyObject* kwds) -> PyObject* {
    PyQuery* self{reinterpret_cast<PyQuery*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->query = nullptr;
    return reinterpret_cast<PyObject*>(self);
}

auto PyQuery_init(PyQuery* self, PyObject* args, PyObject* kwds) -> int {
    assert(nullptr == self->query);

    int py_use_and;
    int py_case_sensitive;
    PyObject* py_query_list;
    if (!PyArg_ParseTuple(
                args,
                "ppO!",
                &py_use_and,
                &py_case_sensitive,
                &PyList_Type,
                &py_query_list)) {
        return -1;
    }

    bool const use_and{(1 == py_use_and) ? true : false};
    bool const case_sensitive{(1 == py_case_sensitive) ? true : false};

    self->query = new Query(use_and, case_sensitive);
    if (nullptr == self->query) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return -1;
    }

    // Note: we don't have to deallocate self->query because dealloc function
    // will handle memory management
    Py_ssize_t const list_size{PyList_Size(py_query_list)};
    for (Py_ssize_t i{0}; i < list_size; ++i) {
        PyObject* item{PyList_GetItem(py_query_list, i)};
        if (!PyUnicode_Check(item)) {
            PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
            return -1;
        }

        char const* wildcard{PyUnicode_AsUTF8(item)};
        if (nullptr == wildcard) {
            // PyUnicode_AsUTF8 sets the exception
            return -1;
        }

        self->query->add_query(std::string_view(wildcard));
    }

    return 0;
}

void PyQuery_dealloc(PyQuery* self) {
    delete self->query;
    PyObject_Del(self);
}

auto PyQuery_match(PyQuery* self, PyObject* args) -> PyObject* {
    PyObject* message_obj;
    if (false == PyArg_ParseTuple(args, "O", &message_obj)) {
        return nullptr;
    }
    Message* message{nullptr};
    try {
        message = reinterpret_cast<PyMessage*>(message_obj)->message;
    } catch (std::exception const& ex) {
        PyErr_SetString(PyExc_TypeError, ex.what());
        return nullptr;
    }
    assert(message);
    bool const match_result{self->query->matches(*message)};
    if (match_result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyMethodDef PyQuery_method_table[]{
        {"match",
         reinterpret_cast<PyCFunction>(PyQuery_match),
         METH_VARARGS,
         "Match the query with a given message."},
        {nullptr}};

static PyType_Slot PyQuery_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyQuery_dealloc)},
        {Py_tp_methods, PyQuery_method_table},
        {Py_tp_init, reinterpret_cast<void*>(PyQuery_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyQuery_new)},
        {0, nullptr}};

static PyType_Spec PyQuery_type_spec{
        "CLPIRDecoder.Query",
        sizeof(PyQuery),
        0,
        Py_TPFLAGS_DEFAULT,
        PyQuery_slots};

PyTypeObject* PyQueryTy{nullptr};

auto PyQuery_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list) -> bool {
    if (nullptr != PyQueryTy) {
        return false;
    }
    PyQueryTy = reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyQuery_type_spec));
    return add_type(reinterpret_cast<PyObject*>(PyQueryTy), "Query", py_module, object_list);
}
} // namespace clp_ffi_py::decoder
