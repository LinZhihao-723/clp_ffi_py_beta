#ifndef CLP_FFI_PY_PYMETADATA
#define CLP_FFI_PY_PYMETADATA

#include "../Python.hpp"

#include "../../clp/components/core/submodules/json/single_include/nlohmann/json.hpp"
#include "Metadata.hpp"

namespace clp_ffi_py::components {
extern PyType_Spec PyMetadataTy;

struct PyMetadata {
    PyObject_HEAD;
    Metadata* metadata;
};

PyObject* PyMetadata_new (PyTypeObject* type, PyObject* args, PyObject* kwds);
int PyMetadata_init (PyMetadata* self, PyObject* args, PyObject* kwds);
void PyMetadata_dealloc (PyMetadata* self);
PyObject* PyMetadata_is_using_four_byte_encoding (PyMetadata* self);
PyObject* PyMetadata_get_ref_timestamp (PyMetadata* self);
PyObject* PyMetadata_get_timestamp_format (PyMetadata* self);
PyObject* PyMetadata_get_timezone (PyMetadata* self);

char const PyMetadata_init_from_json_key[] = "_PyMetadata_init_from_json";
PyMetadata* PyMetadata_init_from_json (nlohmann::json const& metadata, bool is_four_byte_encoding);
using PyMetadataCreateFuncType = decltype(&PyMetadata_init_from_json);
} // namespace clp_ffi_py::components

#endif