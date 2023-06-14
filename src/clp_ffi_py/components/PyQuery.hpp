#ifndef CLP_FFI_PY_PYQUERY
#define CLP_FFI_PY_PYQUERY

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/components/Query.hpp>

namespace clp_ffi_py::components {
extern PyType_Spec PyQueryTy;

struct PyQuery {
    PyObject_HEAD;
    Query* query;
};

auto PyQuery_new (PyTypeObject* type, PyObject* args, PyObject* kwds) -> PyObject*;
auto PyQuery_init (PyQuery* self, PyObject* args, PyObject* kwds) -> int;
void PyQuery_dealloc (PyQuery* self);
auto PyQuery_match (PyQuery* self, PyObject* args) -> PyObject*;
} // namespace clp_ffi_py::components
#endif
