#ifndef CLP_FFI_PY_DECODING_METHODS
#define CLP_FFI_PY_DECODING_METHODS

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/decoder/PyMessage.hpp>
#include <clp_ffi_py/decoder/PyMetadata.hpp>

namespace clp_ffi_py::decoder::four_byte_decoder {
PyObject* decode_preamble(PyObject* self, PyObject* args);
PyObject* decode_next_message(PyObject* self, PyObject* args);
PyObject* decode_next_message_with_query(PyObject* self, PyObject* args);
} // namespace clp_ffi_py::decoder::four_byte_decoder

#endif
