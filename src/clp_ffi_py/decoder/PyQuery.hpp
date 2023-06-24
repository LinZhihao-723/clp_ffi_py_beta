#ifndef CLP_FFI_PY_PYQUERY
#define CLP_FFI_PY_PYQUERY

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/decoder/Query.hpp>

namespace clp_ffi_py::decoder {
struct PyQuery {
    PyObject_HEAD;
    Query* query;
};

auto PyQuery_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list) -> bool;
auto PyQuery_get_PyType() -> PyTypeObject*;
} // namespace clp_ffi_py::decoder
#endif
