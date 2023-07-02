#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/Py_utils.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py {
static std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> Py_get_formatted_timestamp;
static std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> Py_get_timezone_from_timezone_id;

static inline auto Py_utils_function_call(PyObject* func, PyObject* args) -> PyObject* {
    assert(func);
    return PyObject_CallObject(func, args);
}

auto Py_utils_get_formatted_timestamp(PyObject* args) -> PyObject* {
    return Py_utils_function_call(Py_get_formatted_timestamp.get(), args);
}

auto Py_utils_get_timezone_from_timezone_id(PyObject* args) -> PyObject* {
    return Py_utils_function_call(Py_get_timezone_from_timezone_id.get(), args);
}

auto Py_utils_init() -> bool {
    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> utils_module(
            PyImport_ImportModule("clp_ffi_py.utils"));
    auto py_utils{utils_module.get()};
    if (nullptr == py_utils) {
        return false;
    }
    Py_get_timezone_from_timezone_id.reset(
            PyObject_GetAttrString(py_utils, "get_timezone_from_timezone_id"));
    if (nullptr == Py_get_timezone_from_timezone_id.get()) {
        return false;
    }
    Py_get_formatted_timestamp.reset(
            PyObject_GetAttrString(py_utils, "get_formatted_timestamp"));
    if (nullptr == Py_get_formatted_timestamp.get()) {
        return false;
    }
    return true;
}
}