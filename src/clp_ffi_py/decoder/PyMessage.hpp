#ifndef CLP_FFI_PY_PYMESSAGE
#define CLP_FFI_PY_PYMESSAGE

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/decoder/Message.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
struct PyMessage {
    PyObject_HEAD;
    Message* message;
};

PyObject* PyMessage_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
void PyMessage_dealloc(PyMessage* self);
PyObject* PyMessage_get_message(PyMessage* self);
PyObject* PyMessage_get_timestamp(PyMessage* self);
PyObject* PyMessage_get_message_idx(PyMessage* self);
PyObject* PyMessage_wildcard_match(PyMessage* self, PyObject* args);
PyObject* PyMessage_wildcard_match_case_sensitive(PyMessage* self, PyObject* args);

PyMessage* PyMessage_create_empty();
auto PyMessage_create_new(std::string message, ffi::epoch_time_ms_t timestamp, size_t message_idx)
        -> PyMessage*;
auto PyMessageTy_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool;
auto PyMessage_get_PyType(bool init = false) -> PyTypeObject*;
} // namespace clp_ffi_py::decoder

#endif
