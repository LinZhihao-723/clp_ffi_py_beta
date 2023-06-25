#ifndef CLP_FFI_PY_PYMETADATA
#define CLP_FFI_PY_PYMETADATA

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>
#include <clp_ffi_py/decoder/Metadata.hpp>

namespace clp_ffi_py::decoder {
struct PyMetadata {
    PyObject_HEAD;
    Metadata* metadata;
    PyObject* Py_timezone;
};

auto PyMetadata_init_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding)
        -> PyMetadata*;
auto PyMetadata_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list) -> bool;
auto PyMetadata_get_PyType() -> PyTypeObject*;
} // namespace clp_ffi_py::decoder

#endif
