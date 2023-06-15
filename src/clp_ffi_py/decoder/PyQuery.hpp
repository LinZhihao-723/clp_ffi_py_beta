#ifndef CLP_FFI_PY_PYQUERY
#define CLP_FFI_PY_PYQUERY

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/decoder/Query.hpp>

namespace clp_ffi_py::decoder {
struct PyQuery {
    PyObject_HEAD;
    Query* query;
};

extern PyTypeObject* PyQueryTy;

auto PyQuery_new(PyTypeObject* type, PyObject* args, PyObject* kwds) -> PyObject*;
auto PyQuery_init(PyQuery* self, PyObject* args, PyObject* kwds) -> int;
void PyQuery_dealloc(PyQuery* self);
auto PyQuery_match(PyQuery* self, PyObject* args) -> PyObject*;

auto PyQuery_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list) -> bool;
} // namespace clp_ffi_py::decoder
#endif
