#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyMessage.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/decoder/Message.hpp>

namespace clp_ffi_py::decoder {
static std::unique_ptr<PyTypeObject, PyObjectDeleter<PyTypeObject>> PyMessage_type;

static auto get_formatted_timestamp_as_PyString(ffi::epoch_time_ms_t ts, PyObject* timezone)
        -> PyObject* {
    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> func_args_ptr{
            Py_BuildValue("LO", ts, timezone)};
    auto func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    return clp_ffi_py::Py_utils_get_formatted_timestamp(func_args);
}

static auto get_formatted_message(PyMessage* self, PyObject* timezone) -> PyObject* {
    auto cache_formatted_timestamp{false};
    if (Py_None == timezone) {
        if (self->message->has_formatted_timestamp()) {
            // If the formatted timestamp exists, it constructs the raw message
            // without calling python level formatter
            return PyUnicode_FromFormat(
                    "%s%s",
                    self->message->get_formatted_timestamp().c_str(),
                    self->message->get_message().c_str());
        } else if (reinterpret_cast<PyMetadata*>(Py_None) != self->Py_metadata) {
            timezone = self->Py_metadata->Py_timezone;
            cache_formatted_timestamp = true;
        }
    }

    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> timestamp_PyString_ptr{
            get_formatted_timestamp_as_PyString(self->message->get_timestamp(), timezone)};
    auto timestamp_PyString{timestamp_PyString_ptr.get()};
    if (nullptr == timestamp_PyString) {
        return nullptr;
    }
    std::string formatted_timestamp;
    if (false == parse_PyString(timestamp_PyString, formatted_timestamp)) {
        return nullptr;
    }
    if (cache_formatted_timestamp) {
        self->message->set_formatted_timestamp(formatted_timestamp);
    }
    return PyUnicode_FromFormat(
            "%s%s",
            formatted_timestamp.c_str(),
            self->message->get_message().c_str());
}

extern "C" {
static auto PyMessage_init(PyMessage* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_message[] = "message";
    static char keyword_timestamp[] = "timestamp";
    static char keyword_message_idx[] = "message_idx";
    static char keyword_metadata[] = "metadata";
    static char* keyword_table[]{
            static_cast<char*>(keyword_message),
            static_cast<char*>(keyword_timestamp),
            static_cast<char*>(keyword_message_idx),
            static_cast<char*>(keyword_metadata),
            nullptr};

    char const* message_data;
    ffi::epoch_time_ms_t timestamp;
    size_t message_idx{0};
    PyObject* metadata{Py_None};
    if (false == PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "sL|KO",
                         keyword_table,
                         &message_data,
                         &timestamp,
                         &message_idx,
                         &metadata)) {
        return -1;
    }

    self->Py_metadata = nullptr;
    self->message = nullptr;

    std::string message{message_data};
    self->message = new Message(message, timestamp, message_idx);
    if (nullptr == self->message) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        std::cerr << "WTF?\n";
        return -1;
    }
    if (Py_None != metadata && false == PyObject_TypeCheck(metadata, PyMetadata_get_PyType())) {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
        return -1;
    }
    self->set_metadata(reinterpret_cast<PyMetadata*>(metadata));
    return 0;
}

static void PyMessage_dealloc(PyMessage* self) {
    delete self->message;
    Py_XDECREF(self->Py_metadata);
    PyObject_Del(self);
}

static auto PyMessage_get_message(PyMessage* self) -> PyObject* {
    assert(self->message);
    return PyUnicode_FromString(self->message->get_message().c_str());
}

static auto PyMessage_get_timestamp(PyMessage* self) -> PyObject* {
    assert(self->message);
    return PyLong_FromLongLong(self->message->get_timestamp());
}

static auto PyMessage_get_message_idx(PyMessage* self) -> PyObject* {
    assert(self->message);
    return PyLong_FromLongLong(self->message->get_message_idx());
}

static auto PyMessage_wildcard_match(PyMessage* self, PyObject* args) -> PyObject* {
    char const* input_wildcard;
    Py_ssize_t input_wildcard_size;
    if (0 == PyArg_ParseTuple(args, "s#", &input_wildcard, &input_wildcard_size)) {
        return nullptr;
    }
    std::string_view wildcard{input_wildcard, static_cast<size_t>(input_wildcard_size)};
    if (self->message->wildcard_match(wildcard)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static auto PyMessage_wildcard_match_case_sensitive(PyMessage* self, PyObject* args) -> PyObject* {
    char const* input_wildcard;
    Py_ssize_t input_wildcard_size;
    if (0 == PyArg_ParseTuple(args, "s#", &input_wildcard, &input_wildcard_size)) {
        return nullptr;
    }
    std::string_view wildcard{input_wildcard, static_cast<size_t>(input_wildcard_size)};
    if (self->message->wildcard_match_case_sensitive(wildcard)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static auto PyMessage_get_raw_message(PyMessage* self, PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_timezone[] = "timezone";
    static char* key_table[] = {static_cast<char*>(keyword_timezone), nullptr};

    PyObject* timezone{Py_None};
    if (0 == PyArg_ParseTupleAndKeywords(args, keywords, "|O", key_table, &timezone)) {
        return nullptr;
    }

    return get_formatted_message(self, timezone);
}

static auto PyMessage___getstate__(PyMessage* self) -> PyObject* {
    assert(self->message);
    if (false == self->message->has_formatted_timestamp()) {
        PyObject* timezone{self->has_metadata() ? self->Py_metadata->Py_timezone : Py_None};
        std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> timestamp_PyString_ptr{
                get_formatted_timestamp_as_PyString(self->message->get_timestamp(), timezone)};
        auto timestamp_PyString{timestamp_PyString_ptr.get()};
        if (nullptr == timestamp_PyString) {
            return nullptr;
        }
        char const* timestamp_str{PyUnicode_AsUTF8(timestamp_PyString)};
        std::string formatted_timestamp{std::string(timestamp_str)};
        self->message->set_formatted_timestamp(formatted_timestamp);
    }

    return Py_BuildValue(
            "{sssssLsK}",
            cStateMessage,
            self->message->get_message().c_str(),
            cStateFormattedTimestamp,
            self->message->get_formatted_timestamp().c_str(),
            cStateTimestamp,
            self->message->get_timestamp(),
            cStateMessageIdx,
            self->message->get_message_idx());
}

static auto PyMessage___setstate__(PyMessage* self, PyObject* state) -> PyObject* {
    if (false == PyDict_CheckExact(state)) {
        PyErr_SetString(PyExc_ValueError, clp_ffi_py::error_messages::pickled_state_error);
        return nullptr;
    }

    auto message_obj{PyDict_GetItemString(state, cStateMessage)};
    if (nullptr == message_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateMessage);
        return nullptr;
    }
    std::string message;
    if (false == parse_PyString(message_obj, message)) {
        return nullptr;
    }

    auto formatted_timestamp_obj{PyDict_GetItemString(state, cStateFormattedTimestamp)};
    if (nullptr == formatted_timestamp_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateFormattedTimestamp);
        return nullptr;
    }
    std::string formatted_timestamp;
    if (false == parse_PyString(formatted_timestamp_obj, formatted_timestamp)) {
        return nullptr;
    }

    auto timestamp_obj{PyDict_GetItemString(state, cStateTimestamp)};
    if (nullptr == timestamp_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateTimestamp);
        return nullptr;
    }
    ffi::epoch_time_ms_t timestamp;
    if (false == parse_PyInt<ffi::epoch_time_ms_t>(timestamp_obj, timestamp)) {
        return nullptr;
    }

    auto message_idx_obj{PyDict_GetItemString(state, cStateMessageIdx)};
    if (nullptr == message_idx_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateMessageIdx);
        return nullptr;
    }
    size_t message_idx;
    if (false == parse_PyInt<size_t>(message_idx_obj, message_idx)) {
        return nullptr;
    }

    self->message = new Message(message, formatted_timestamp, timestamp, message_idx);
    if (nullptr == self->message) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }

    Py_RETURN_NONE;
}

static auto PyMessage___str__(PyMessage* self) -> PyObject* {
    return get_formatted_message(self, Py_None);
}

static auto PyMessage___repr__(PyMessage* self) -> PyObject* {
    return PyObject_Repr(PyMessage___getstate__(self));
}
}

auto PyMessage_create_new(
        std::string message,
        ffi::epoch_time_ms_t timestamp,
        size_t message_idx,
        PyMetadata* metadata) -> PyMessage* {
    PyMessage* self{reinterpret_cast<PyMessage*>(PyObject_New(PyMessage, PyMessage_get_PyType()))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }

    self->Py_metadata = nullptr;
    self->message = new Message(message, timestamp, message_idx);
    if (nullptr == self->message) {
        Py_DECREF(self);
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }
    self->set_metadata(metadata);
    return self;
}

static PyMemberDef PyMessage_members[] = {
        {"metadata",
         T_OBJECT,
         offsetof(PyMessage, Py_metadata),
         READONLY,
         "Message metadata binding"},
        {nullptr}};

static PyMethodDef PyMessage_method_table[]{
        {"get_message",
         reinterpret_cast<PyCFunction>(PyMessage_get_message),
         METH_NOARGS,
         "Get message as a string."},

        {"get_timestamp",
         reinterpret_cast<PyCFunction>(PyMessage_get_timestamp),
         METH_NOARGS,
         "Get timestamp as a integer."},

        {"get_message_idx",
         reinterpret_cast<PyCFunction>(PyMessage_get_message_idx),
         METH_NOARGS,
         "Get message index as a integer."},

        {"wildcard_match",
         reinterpret_cast<PyCFunction>(PyMessage_wildcard_match),
         METH_VARARGS,
         "Wildcard match (case insensitive)"},

        {"wildcard_match_case_sensitive",
         reinterpret_cast<PyCFunction>(PyMessage_wildcard_match_case_sensitive),
         METH_VARARGS,
         "Wildcard match (case sensitive)"},

        {"get_raw_message",
         reinterpret_cast<PyCFunction>(PyMessage_get_raw_message),
         METH_KEYWORDS | METH_VARARGS,
         "Get the raw message by formatting timestamp and message contents"},

        {"__getstate__",
         reinterpret_cast<PyCFunction>(PyMessage___getstate__),
         METH_NOARGS,
         "Pickle the Message object"},

        {"__setstate__",
         reinterpret_cast<PyCFunction>(PyMessage___setstate__),
         METH_O,
         "Un-pickle the Message object"},

        {nullptr}};

static PyType_Slot PyMessage_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMessage_dealloc)},
        {Py_tp_methods, PyMessage_method_table},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_members, PyMessage_members},
        {Py_tp_init, reinterpret_cast<void*>(PyMessage_init)},
        {Py_tp_str, reinterpret_cast<void*>(PyMessage___str__)},
        {Py_tp_repr, reinterpret_cast<void*>(PyMessage___repr__)},
        {0, nullptr}};

static PyType_Spec PyMessage_type_spec{
        "clp_ffi_py.CLPIRDecoder.Message",
        sizeof(PyMessage),
        0,
        Py_TPFLAGS_DEFAULT,
        PyMessage_slots};

auto PyMessage_get_PyType() -> PyTypeObject* {
    return PyMessage_type.get();
}

auto PyMessageTy_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool {
    auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMessage_type_spec))};
    PyMessage_type.reset(type);
    return add_type(
            reinterpret_cast<PyObject*>(PyMessage_get_PyType()),
            "Message",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
