#ifndef CLP_FFI_PY_PYOBJECTDELETER
#define CLP_FFI_PY_PYOBJECTDELETER

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <memory>

namespace clp_ffi_py {
template <typename PyObjectType>
class PyObjectDeleter {
public:
    void operator()(PyObjectType* ptr) { Py_XDECREF(reinterpret_cast<PyObject*>(ptr)); }
};
} // namespace clp_ffi_py

#endif
