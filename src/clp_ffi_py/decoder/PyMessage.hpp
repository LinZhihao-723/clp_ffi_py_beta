#ifndef CLP_FFI_PY_PYMESSAGE
#define CLP_FFI_PY_PYMESSAGE

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/decoder/Message.hpp>
#include <clp_ffi_py/decoder/PyMetadata.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
struct PyMessage {
    PyObject_HEAD;
    Message* message;
    PyMetadata* Py_metadata;

    [[nodiscard]] bool has_metadata() { return nullptr != Py_metadata; }

    void set_metadata(PyMetadata* metadata) {
        Py_XDECREF(Py_metadata);
        Py_metadata = metadata;
        Py_INCREF(Py_metadata);
    }
};

constexpr char cStateMessage[] = "message";
constexpr char cStateFormattedTimestamp[] = "formatted_timestamp";
constexpr char cStateTimestamp[] = "timestamp";
constexpr char cStateMessageIdx[] = "message_idx";

auto PyMessage_create_new(
        std::string message,
        ffi::epoch_time_ms_t timestamp,
        size_t message_idx,
        PyMetadata* metadata) -> PyMessage*;
auto PyMessageTy_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool;
auto PyMessage_get_PyType() -> PyTypeObject*;
} // namespace clp_ffi_py::decoder
#endif
