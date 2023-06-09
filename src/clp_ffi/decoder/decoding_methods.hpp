#ifndef CLP_FFI_PY_DECODING_METHOD
#define CLP_FFI_PY_DECODING_METHOD

#include "../Python.hpp"

#include "../components/PyMessage.hpp"
#include "../components/PyMetadata.hpp"

namespace clp_ffi_py::decoder::four_byte_decoder {
extern clp_ffi_py::components::PyMessageCreateFuncType PyMessage_create_empty;
extern clp_ffi_py::components::PyMetadataCreateFuncType PyMetadata_init_from_json;

PyObject* decode_preamble (PyObject* self, PyObject* args);
PyObject* decode_next_message (PyObject* self, PyObject* args);
PyObject* decode_next_message_with_query (PyObject* self, PyObject* args);
} // namespace clp_ffi_py::decoder::four_byte_decoder

#endif
