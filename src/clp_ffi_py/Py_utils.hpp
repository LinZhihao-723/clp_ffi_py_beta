#ifndef PY_UTILS_HPP
#define PY_UTILS_HPP

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

namespace clp_ffi_py {
auto Py_utils_get_formatted_timestamp(PyObject* args) -> PyObject*;
auto Py_utils_get_timezone_from_timezone_id(PyObject* args) -> PyObject*;
auto Py_utils_init() -> bool;
}

#endif