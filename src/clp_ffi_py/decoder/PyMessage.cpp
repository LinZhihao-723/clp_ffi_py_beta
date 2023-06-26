#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyMessage.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/decoder/Message.hpp>

namespace clp_ffi_py::decoder {
void PyMessage::set_metadata(PyMetadata* metadata) {
    if (reinterpret_cast<PyMetadata*>(Py_None) != Py_metadata) {
        Py_DECREF(Py_metadata);
    }
    Py_metadata = reinterpret_cast<PyMetadata*>(metadata);
    Py_INCREF(Py_metadata);
}

static std::unique_ptr<PyTypeObject, PyObjectDeleter<PyTypeObject>> PyMessage_type;
static std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> Py_utils_get_formatted_timestamp;

static auto get_formatted_timestamp_as_PyString(ffi::epoch_time_ms_t ts, PyObject* timezone)
        -> PyObject* {
    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> func_args_ptr{
            Py_BuildValue("LO", ts, timezone)};
    auto func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    return PyObject_CallObject(Py_utils_get_formatted_timestamp.get(), func_args);
}

extern "C" {
static auto PyMessage_new(PyTypeObject* type, PyObject* args, PyObject* keywords) -> PyObject* {
    PyMessage* self{reinterpret_cast<PyMessage*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->Py_metadata = reinterpret_cast<PyMetadata*>(Py_None);
    self->message = new Message();
    if (nullptr == self->message) {
        Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    return reinterpret_cast<PyObject*>(self);
}

static void PyMessage_dealloc(PyMessage* self) {
    delete self->message;
    Py_XDECREF(self->Py_metadata);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
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
        }
    }

    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> timestamp_PyString_ptr{
            get_formatted_timestamp_as_PyString(self->message->get_timestamp(), timezone)};
    auto timestamp_PyString{timestamp_PyString_ptr.get()};
    if (nullptr == timestamp_PyString) {
        return nullptr;
    }
    return PyUnicode_FromFormat("%S%s", timestamp_PyString, self->message->get_message().c_str());
}

static auto PyMessage_set_metadata(PyMessage* self, PyObject* args) {
    PyObject* metadata{nullptr};
    if (0 == PyArg_ParseTuple(args, "O!", PyMetadata_get_PyType(), &metadata)) {
        return nullptr;
    }
    self->set_metadata(reinterpret_cast<PyMetadata*>(metadata));
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

    // std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> timestamp_PyString_ptr{
    //         get_formatted_timestamp_as_PyString(
    //                 self->message->get_timestamp(),
    //                 metadata->Py_timezone)};
    // auto timestamp_PyString{timestamp_PyString_ptr.get()};
    // if (nullptr == timestamp_PyString) {
    //     return nullptr;
    // }
    // Py_ssize_t timestamp_str_size{0};
    // char const* timestamp_str{PyUnicode_AsUTF8AndSize(timestamp_PyString, &timestamp_str_size)};
    // std::string formatted_timestamp{std::string(timestamp_str, timestamp_str_size)};
    // self->message = new Message(message, formatted_timestamp, timestamp, message_idx);

    self->Py_metadata = reinterpret_cast<PyMetadata*>(Py_None);
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
        {"set_metadata",
         reinterpret_cast<PyCFunction>(PyMessage_set_metadata),
         METH_VARARGS,
         "Set the metadata field"},
        {nullptr}};

static PyType_Slot PyMessage_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMessage_dealloc)},
        {Py_tp_methods, PyMessage_method_table},
        {Py_tp_init, nullptr},
        {Py_tp_new, reinterpret_cast<void*>(PyMessage_new)},
        {Py_tp_members, PyMessage_members},
        {0, nullptr}};

static PyType_Spec PyMessage_type_spec{
        "CLPIRDecoder.Message",
        sizeof(PyMessage),
        0,
        Py_TPFLAGS_DEFAULT,
        PyMessage_slots};

static auto utils_init() -> bool {
    std::unique_ptr<PyObject, PyObjectDeleter<PyObject>> utils_module(
            PyImport_ImportModule("clp_ffi_py.utils"));
    auto py_utils{utils_module.get()};
    if (nullptr == py_utils) {
        return false;
    }
    Py_utils_get_formatted_timestamp.reset(
            PyObject_GetAttrString(py_utils, "get_formatted_timestamp"));
    if (nullptr == Py_utils_get_formatted_timestamp.get()) {
        return false;
    }
    return true;
}

auto PyMessage_get_PyType() -> PyTypeObject* {
    return PyMessage_type.get();
}

auto PyMessageTy_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list)
        -> bool {
    if (false == utils_init()) {
        return false;
    }
    auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMessage_type_spec))};
    PyMessage_type.reset(type);
    if (nullptr != type) {
        Py_INCREF(type);
    }
    return add_type(
            reinterpret_cast<PyObject*>(PyMessage_get_PyType()),
            "Message",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
