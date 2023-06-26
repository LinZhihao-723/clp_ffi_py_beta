#ifndef CLP_FFI_PY_UTILITIES_TPP
#define CLP_FFI_PY_UTILITIES_TPP

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <type_traits>

template <typename int_type>
auto parse_PyInt(PyObject* Py_int, int_type& val) -> bool {
    if (false == PyLong_Check(Py_int)) {
        PyErr_SetString(PyExc_TypeError, "parse_PyInt receives none-int argument.");
    }

    if constexpr (std::is_same_v<int_type, size_t>) {
        val = PyLong_AsSize_t(Py_int);
    } else if constexpr (std::is_same_v<int_type, ffi::epoch_time_ms_t>) {
        val = PyLong_AsLongLong(Py_int);
    } else if constexpr (std::is_same_v<int_type, Py_ssize_t>) {
        val = PyLong_AsSsize_t(Py_int);
    } else {
        PyErr_SetString(PyExc_TypeError, "parse_PyInt receives unsupported int type.");
        return false;
    }

    return (nullptr == PyErr_Occurred());
}

#endif
