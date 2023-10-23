#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyLogEvent.hpp"

#include <iomanip>
#include <sstream>

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/LogEvent.hpp>
#include <clp_ffi_py/ir/native/PyQuery.hpp>
#include <clp_ffi_py/ir/native/utils.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
/**
 * Formats the android attributes.
 * @param attributes
 * @param formatted_attributes
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto format_android_log(
        LogEvent::attribute_table_t const& attributes,
        std::string& formatted_attributes
) -> bool {
    auto get_priority_char = [](ffi::ir_stream::attr_int_t priority) -> char {
        switch (priority) {
                /* clang-format off */
            case 2: return 'V';
            case 3: return 'D';
            case 4: return 'I';
            case 5: return 'W';
            case 6: return 'E';
            case 7: return 'F';
            case 8: return 'S';

            default:                  return '?';
                /* clang-format on */
        }
    };
    try {
        std::ostringstream attribute_formatter;
        attribute_formatter << " " << std::setw(5)
                            << attributes.at("pid").value().get_value<ffi::ir_stream::attr_int_t>();
        attribute_formatter << " " << std::setw(5)
                            << attributes.at("tid").value().get_value<ffi::ir_stream::attr_int_t>();
        auto const priority_val{
                attributes.at("priority").value().get_value<ffi::ir_stream::attr_int_t>()
        };
        attribute_formatter << " " << get_priority_char(priority_val);
        attribute_formatter << " " << std::left << std::setw(8) << std::setfill(' ')
                            << attributes.at("tag").value().get_value<ffi::ir_stream::attr_str_t>();
        attribute_formatter << ": ";
        formatted_attributes = attribute_formatter.str();
    } catch (std::exception const& ex) {
        PyErr_Format(
                PyExc_RuntimeError,
                "Failed to format android logs with attributes. std::exception: %s",
                ex.what()
        );
        return false;
    }
    return true;
}

extern "C" {
/**
 * Callback of PyLogEvent `__init__` method:
 * __init__(log_message, timestamp, index=0, metadata=None)
 * Keyword argument parsing is supported.
 * Assumes `self` is uninitialized and will allocate the underlying memory. If
 * `self` is already initialized this will result in memory leaks.
 * @param self
 * @param args
 * @param keywords
 * @return 0 on success.
 * @return -1 on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_init(PyLogEvent* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_message[]{"log_message"};
    static char keyword_timestamp[]{"timestamp"};
    static char keyword_message_idx[]{"index"};
    static char keyword_metadata[]{"metadata"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_message),
            static_cast<char*>(keyword_timestamp),
            static_cast<char*>(keyword_message_idx),
            static_cast<char*>(keyword_metadata),
            nullptr
    };

    // If the argument parsing fails, `self` will be deallocated. We must reset
    // all pointers to nullptr in advance, otherwise the deallocator might
    // trigger segmentation fault
    self->default_init();

    char const* log_message{nullptr};
    ffi::epoch_time_ms_t timestamp{0};
    size_t index{0};
    PyObject* metadata{Py_None};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "sL|KO",
                static_cast<char**>(keyword_table),
                &log_message,
                &timestamp,
                &index,
                &metadata
        )))
    {
        return -1;
    }

    auto const has_metadata{Py_None != metadata};
    if (has_metadata
        && false == static_cast<bool>(PyObject_TypeCheck(metadata, PyMetadata::get_py_type())))
    {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::cPyTypeError);
        return -1;
    }

    if (false
        == self->init(
                log_message,
                timestamp,
                index,
                has_metadata ? py_reinterpret_cast<PyMetadata>(metadata) : nullptr,
                {}
        ))
    {
        return -1;
    }
    return 0;
}

/**
 * Callback of PyLogEvent deallocator.
 * @param self
 */
auto PyLogEvent_dealloc(PyLogEvent* self) -> void {
    self->clean();
    PyObject_Del(self);
}

/**
 * Constant keys used to serialize/deserialize PyLogEvent objects through
 * __getstate__ and __setstate__ methods.
 */
constexpr char const* const cStateLogMessage = "log_message";
constexpr char const* const cStateTimestamp = "timestamp";
constexpr char const* const cStateFormattedTimestamp = "formatted_timestamp";
constexpr char const* const cStateIndex = "index";
constexpr char const* const cStateAttributes = "attributes";

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetStateDoc,
        "__getstate__(self)\n"
        "--\n\n"
        "Serializes the log event (should be called by the Python pickle module).\n\n"
        ":return: Serialized log event in a Python dictionary.\n"
);

/**
 * Callback of PyLogEvent `__getstate__` method.
 * @param self
 * @return Python dictionary that contains the serialized object.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_getstate(PyLogEvent* self) -> PyObject* {
    auto* log_event{self->get_log_event()};
    if (false == log_event->has_formatted_timestamp()) {
        PyObjectPtr<PyObject> const formatted_timestamp_object{
                clp_ffi_py::py_utils_get_formatted_timestamp(
                        log_event->get_timestamp(),
                        self->has_metadata() ? self->get_py_metadata()->get_py_timezone() : Py_None
                )
        };
        auto* formatted_timestamp_ptr{formatted_timestamp_object.get()};
        if (nullptr == formatted_timestamp_ptr) {
            return nullptr;
        }
        std::string formatted_timestamp;
        if (false == clp_ffi_py::parse_py_string(formatted_timestamp_ptr, formatted_timestamp)) {
            return nullptr;
        }
        if (self->has_metadata() && self->get_py_metadata()->get_metadata()->is_android_log()
            && self->get_log_event()->has_attributes())
        {
            std::string formatted_attributes;
            if (false
                == format_android_log(
                        self->get_log_event()->get_attributes(),
                        formatted_attributes
                ))
            {
                return nullptr;
            }
            formatted_timestamp += formatted_attributes;
        }
        log_event->set_formatted_timestamp(formatted_timestamp);
    }

    auto const& attributes{self->get_log_event()->get_attributes()};
    auto* py_attributes{serialize_attributes_to_python_dict(attributes)};
    if (nullptr == py_attributes) {
        return nullptr;
    }

    return Py_BuildValue(
            "{sssssLsKsO}",
            cStateLogMessage,
            log_event->get_log_message().c_str(),
            static_cast<char const*>(cStateFormattedTimestamp),
            log_event->get_formatted_timestamp().c_str(),
            static_cast<char const*>(cStateTimestamp),
            log_event->get_timestamp(),
            cStateIndex,
            log_event->get_index(),
            cStateAttributes,
            py_attributes
    );
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventSetStateDoc,
        "__setstate__(self, state)\n"
        "--\n\n"
        "Deserializes the log event from a state dictionary.\n\n"
        "Note: this function is exclusively designed for invocation by the Python pickle module. "
        "Assumes `self` is uninitialized and will allocate the underlying memory. If"
        "`self` is already initialized this will result in memory leaks.\n\n"
        ":param state: Serialized log event represented by a Python dictionary. It is anticipated "
        "to be the valid output of the `__getstate__` method.\n"
        ":return: None\n"
);

/**
 * Callback of PyLogEvent `__setstate__` method.
 * Note: should only be used by the Python pickle module.
 * Assumes `self` is uninitialized and will allocate the underlying memory. If
 * `self` is already initialized this will result in memory leaks.
 * @param self
 * @param state Python dictionary that contains the serialized object info.
 * @return Py_None on success
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_setstate(PyLogEvent* self, PyObject* state) -> PyObject* {
    self->default_init();

    if (false == static_cast<bool>(PyDict_CheckExact(state))) {
        PyErr_SetString(PyExc_ValueError, clp_ffi_py::cSetstateInputError);
        return nullptr;
    }

    auto* log_message_obj{PyDict_GetItemString(state, cStateLogMessage)};
    if (nullptr == log_message_obj) {
        PyErr_Format(PyExc_KeyError, clp_ffi_py::cSetstateKeyErrorTemplate, cStateLogMessage);
        return nullptr;
    }
    std::string log_message;
    if (false == clp_ffi_py::parse_py_string(log_message_obj, log_message)) {
        return nullptr;
    }

    auto* formatted_timestamp_obj{PyDict_GetItemString(state, cStateFormattedTimestamp)};
    if (nullptr == formatted_timestamp_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::cSetstateKeyErrorTemplate,
                cStateFormattedTimestamp
        );
        return nullptr;
    }
    std::string formatted_timestamp;
    if (false == clp_ffi_py::parse_py_string(formatted_timestamp_obj, formatted_timestamp)) {
        return nullptr;
    }

    auto* timestamp_obj{PyDict_GetItemString(state, cStateTimestamp)};
    if (nullptr == timestamp_obj) {
        PyErr_Format(PyExc_KeyError, clp_ffi_py::cSetstateKeyErrorTemplate, cStateTimestamp);
        return nullptr;
    }
    ffi::epoch_time_ms_t timestamp{0};
    if (false == clp_ffi_py::parse_py_int<ffi::epoch_time_ms_t>(timestamp_obj, timestamp)) {
        return nullptr;
    }

    auto* index_obj{PyDict_GetItemString(state, cStateIndex)};
    if (nullptr == index_obj) {
        PyErr_Format(PyExc_KeyError, clp_ffi_py::cSetstateKeyErrorTemplate, cStateIndex);
        return nullptr;
    }
    size_t index{0};
    if (false == clp_ffi_py::parse_py_int<size_t>(index_obj, index)) {
        return nullptr;
    }

    LogEvent::attribute_table_t attributes;
    auto* attributes_obj{PyDict_GetItemString(state, cStateAttributes)};
    if (nullptr != attributes_obj
        && false == deserialize_attributes_from_python_dict(attributes_obj, attributes))
    {
        return nullptr;
    }

    if (false
        == self->init(log_message, timestamp, index, nullptr, attributes, formatted_timestamp))
    {
        return nullptr;
    }

    Py_RETURN_NONE;
}

/**
 * Callback of PyLogEvent `__str__` method.
 * @param self
 * @return PyLogEvent::get_formatted_log_message
 */
auto PyLogEvent_str(PyLogEvent* self) -> PyObject* {
    return self->get_formatted_message();
}

/**
 * Callback of PyLogEvent `__repr__` method.
 * @param self
 * @return Python string representation of PyLogEvent state.
 */
auto PyLogEvent_repr(PyLogEvent* self) -> PyObject* {
    return PyObject_Repr(PyLogEvent_getstate(self));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetLogMessageDoc,
        "get_log_message(self)\n"
        "--\n\n"
        "Gets the log message of the log event.\n\n"
        ":return: The log message.\n"
);

auto PyLogEvent_get_log_message(PyLogEvent* self) -> PyObject* {
    return PyUnicode_FromString(self->get_log_event()->get_log_message().c_str());
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetTimestampDoc,
        "get_timestamp(self)\n"
        "--\n\n"
        "Gets the Unix epoch timestamp in milliseconds of the log event.\n\n"
        ":return: The timestamp in milliseconds.\n"
);

auto PyLogEvent_get_timestamp(PyLogEvent* self) -> PyObject* {
    return PyLong_FromLongLong(self->get_log_event()->get_timestamp());
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetIndexDoc,
        "get_index(self)\n"
        "--\n\n"
        "Gets the message index (relative to the source CLP IR stream) of the log event. This "
        "index is set to 0 by default.\n\n"
        ":return: The log event index.\n"
);

auto PyLogEvent_get_index(PyLogEvent* self) -> PyObject* {
    return PyLong_FromLongLong(static_cast<int64_t>(self->get_log_event()->get_index()));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventMatchQueryDoc,
        "match_query(self, query)\n"
        "--\n\n"
        "Matches the underlying log event against the given query. Refer to the documentation of "
        "clp_ffi_py.Query for more details.\n\n"
        ":param query: Input Query object.\n"
        ":return: True if the log event matches the query, False otherwise.\n"
);

auto PyLogEvent_match_query(PyLogEvent* self, PyObject* query) -> PyObject* {
    if (false == static_cast<bool>(PyObject_TypeCheck(query, PyQuery::get_py_type()))) {
        PyErr_SetString(PyExc_TypeError, cPyTypeError);
        return nullptr;
    }
    auto* py_query{py_reinterpret_cast<PyQuery>(query)};
    return get_py_bool(py_query->get_query()->matches(*self->get_log_event()));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetFormattedMessageDoc,
        "get_formatted_message(self, timezone=None)\n"
        "--\n\n"
        "Gets the formatted log message of the log event.\n\n"
        "If a specific timezone is provided, it will be used to format the timestamp. "
        "Otherwise, the timestamp will be formatted using the timezone from the source CLP IR "
        "stream.\n\n"
        ":param timezone: Python tzinfo object that specifies a timezone.\n"
        ":return: The formatted message.\n"
);

auto PyLogEvent_get_formatted_message(PyLogEvent* self, PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_timezone[]{"timezone"};
    static char* key_table[]{static_cast<char*>(keyword_timezone), nullptr};

    PyObject* timezone{Py_None};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "|O",
                static_cast<char**>(key_table),
                &timezone
        )))
    {
        return nullptr;
    }

    return self->get_formatted_message(timezone);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetAttributesDoc,
        "get_attributes()\n"
        "--\n\n"
        ":return: The attributes associated with the log returned as a newly created Python "
        "dictionary. Each attribute is stored as a key value pair.\n"
);

auto PyLogEvent_get_attributes(PyLogEvent* self) -> PyObject* {
    return serialize_attributes_to_python_dict(self->get_log_event()->get_attributes());
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventGetCachedEncodedLogEventDoc,
        "get_cached_encoded_log_event()\n"
        "--\n\n"
        ":return: A bytearray that contains the encoded log event.\n"
        ":return: None if the encoded log event is not cached.\n"
);

auto PyLogEvent_get_cached_encoded_log_event(PyLogEvent* self) -> PyObject* {
    auto const* log_event{self->get_log_event()};
    if (false == log_event->has_cached_encoded_log_event()) {
        Py_RETURN_NONE;
    }
    auto encoded_log_event_view{log_event->get_cached_encoded_log_event()};
    return PyByteArray_FromStringAndSize(
            size_checked_pointer_cast<char const>(encoded_log_event_view.data()),
            static_cast<Py_ssize_t>(encoded_log_event_view.size())
    );
}
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyLogEvent_method_table[]{
        {"get_log_message",
         py_c_function_cast(PyLogEvent_get_log_message),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetLogMessageDoc)},

        {"get_timestamp",
         py_c_function_cast(PyLogEvent_get_timestamp),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetTimestampDoc)},

        {"get_index",
         py_c_function_cast(PyLogEvent_get_index),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetIndexDoc)},

        {"get_attributes",
         py_c_function_cast(PyLogEvent_get_attributes),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetAttributesDoc)},

        {"get_formatted_message",
         py_c_function_cast(PyLogEvent_get_formatted_message),
         METH_KEYWORDS | METH_VARARGS,
         static_cast<char const*>(cPyLogEventGetFormattedMessageDoc)},

        {"get_cached_encoded_log_event",
         py_c_function_cast(PyLogEvent_get_cached_encoded_log_event),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetCachedEncodedLogEventDoc)},

        {"match_query",
         py_c_function_cast(PyLogEvent_match_query),
         METH_O,
         static_cast<char const*>(cPyLogEventMatchQueryDoc)},

        {"__getstate__",
         py_c_function_cast(PyLogEvent_getstate),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetStateDoc)},

        {"__setstate__",
         py_c_function_cast(PyLogEvent_setstate),
         METH_O,
         static_cast<char const*>(cPyLogEventSetStateDoc)},

        {nullptr}
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyLogEventDoc,
        "This class represents a decoded log event and provides ways to access the underlying "
        "log data, including the log message, the timestamp, and the log event index. "
        "Normally, this class will be instantiated by the FFI IR decoding methods.\n"
        "However, with the `__init__` method provided below, direct instantiation is also "
        "possible.\n\n"
        "The signature of `__init__` method is shown as following:\n\n"
        "__init__(self, log_message, timestamp, index=0, metadata=None)\n\n"
        "Initializes an object that represents a log event. Notice that each object should be "
        "strictly initialized only once. Double initialization will result in memory leaks.\n\n"
        ":param log_message: The message content of the log event.\n"
        ":param timestamp: The timestamp of the log event.\n"
        ":param index: The message index (relative to the source CLP IR stream) of the log event.\n"
        ":param metadata: The PyMetadata instance that represents the source CLP IR stream. "
        "It is set to None by default.\n"
);

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
PyType_Slot PyLogEvent_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyLogEvent_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyLogEvent_init)},
        {Py_tp_str, reinterpret_cast<void*>(PyLogEvent_str)},
        {Py_tp_repr, reinterpret_cast<void*>(PyLogEvent_repr)},
        {Py_tp_methods, static_cast<void*>(PyLogEvent_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyLogEventDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * PyLogEvent Python type specifications.
 */
PyType_Spec PyLogEvent_type_spec{
        "clp_ffi_py.ir.native.LogEvent",
        sizeof(PyLogEvent),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyLogEvent_slots)
};
}  // namespace

auto PyLogEvent::get_formatted_message(PyObject* timezone) -> PyObject* {
    auto cache_formatted_timestamp{false};
    if (Py_None == timezone) {
        if (m_log_event->has_formatted_timestamp()) {
            // If the formatted timestamp exists, it constructs the raw message
            // without calling python level format function
            return PyUnicode_FromFormat(
                    "%s%s",
                    m_log_event->get_formatted_timestamp().c_str(),
                    m_log_event->get_log_message().c_str()
            );
        }
        if (has_metadata()) {
            timezone = m_py_metadata->get_py_timezone();
            cache_formatted_timestamp = true;
        }
    }

    PyObjectPtr<PyObject> const formatted_timestamp_object{
            py_utils_get_formatted_timestamp(m_log_event->get_timestamp(), timezone)
    };
    auto* formatted_timestamp_ptr{formatted_timestamp_object.get()};
    if (nullptr == formatted_timestamp_ptr) {
        return nullptr;
    }
    std::string formatted_timestamp;
    if (false == parse_py_string(formatted_timestamp_ptr, formatted_timestamp)) {
        return nullptr;
    }
    if (has_metadata() && m_py_metadata->get_metadata()->is_android_log()
        && m_log_event->has_attributes())
    {
        std::string formatted_attributes;
        if (false == format_android_log(m_log_event->get_attributes(), formatted_attributes)) {
            return nullptr;
        }
        formatted_timestamp += formatted_attributes;
    }

    if (cache_formatted_timestamp) {
        m_log_event->set_formatted_timestamp(formatted_timestamp);
    }
    return PyUnicode_FromFormat(
            "%s%s",
            formatted_timestamp.c_str(),
            m_log_event->get_log_message().c_str()
    );
}

auto PyLogEvent::init(
        std::string_view log_message,
        ffi::epoch_time_ms_t timestamp,
        size_t index,
        PyMetadata* metadata,
        LogEvent::attribute_table_t const& attributes,
        std::optional<std::string_view> formatted_timestamp,
        std::optional<gsl::span<int8_t>> encoded_log_event_view
) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    m_log_event = new LogEvent(
            log_message,
            timestamp,
            index,
            attributes,
            formatted_timestamp,
            encoded_log_event_view
    );
    if (nullptr == m_log_event) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::cOutofMemoryError);
        return false;
    }
    set_metadata(metadata);
    return true;
}

PyObjectGlobalPtr<PyTypeObject> PyLogEvent::m_py_type{nullptr};

auto PyLogEvent::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyLogEvent::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyLogEvent>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyLogEvent_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_python_type(get_py_type(), "LogEvent", py_module);
}

auto PyLogEvent::create_new_log_event(
        std::string const& log_message,
        ffi::epoch_time_ms_t timestamp,
        size_t index,
        PyMetadata* metadata,
        LogEvent::attribute_table_t const& attributes,
        std::optional<gsl::span<int8_t>> encoded_log_event_view
) -> PyLogEvent* {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    PyLogEvent* self{PyObject_New(PyLogEvent, get_py_type())};
    if (nullptr == self) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::cOutofMemoryError);
        return nullptr;
    }
    self->default_init();
    if (false
        == self->init(
                log_message,
                timestamp,
                index,
                metadata,
                attributes,
                std::nullopt,
                encoded_log_event_view
        ))
    {
        return nullptr;
    }
    return self;
}
}  // namespace clp_ffi_py::ir::native
