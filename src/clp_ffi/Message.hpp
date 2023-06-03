#ifndef CLP_PY_MESSAGE
#define CLP_PY_MESSAGE

#include <Python.h>

#include "../clp/components/core/src/ffi/encoding_methods.hpp"

#include "Metadata.hpp"

namespace clp_ffi_py::message {
class Message {
public:
    explicit Message() = default;

    std::string& get_message_ref () { return m_message; }
    ffi::epoch_time_ms_t& get_timestamp_ref () { return m_timestamp; }

private:
    std::string m_message;
    ffi::epoch_time_ms_t m_timestamp;
};

extern PyType_Spec PyMessageTy;

struct PyMessage {
    PyObject_HEAD;
    Message* message;
};

PyObject* PyMessage_new (PyTypeObject* type, PyObject* args, PyObject* kwds);
void PyMessage_dealloc (PyMessage* self);
PyObject* PyMessage_get_message (PyMessage* self);
PyObject* PyMessage_get_timestamp (PyMessage* self);

PyMessage* PyMessage_create ();
} // namespace clp_ffi_py::message

#endif