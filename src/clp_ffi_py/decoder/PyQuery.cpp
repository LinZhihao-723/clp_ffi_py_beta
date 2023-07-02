#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyQuery.hpp>

#include <clp/components/core/src/ffi/encoding_methods.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/decoder/PyMessage.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
static auto serialize_query_list(Query& query) -> PyObject* {
    auto& query_list{query.get_query_list_const_ref()};
    auto const query_list_size{query_list.size()};

    auto py_query_list{PyList_New(query_list_size)};
    if (nullptr == py_query_list) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }

    Py_ssize_t idx{0};
    std::vector<PyObject*> py_query_object_list;
    for (auto const& query_wildcard : query_list) {
        PyObject* py_str = PyUnicode_FromString(query_wildcard.c_str());
        if (nullptr == py_str) {
            PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
            for (auto object : py_query_object_list) {
                Py_DECREF(object);
            }
            return nullptr;
        }
        py_query_object_list.push_back(py_str);
        PyList_SET_ITEM(py_query_list, idx, py_str);
        ++idx;
    }

    return py_query_list;
}

static auto deserialize_query_list(Query& query, PyObject* list) -> bool {
    if (false == PyObject_TypeCheck(list, &PyList_Type)) {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
        return false;
    }
    Py_ssize_t const list_size{PyList_Size(list)};
    for (Py_ssize_t i{0}; i < list_size; ++i) {
        PyObject* wildcard{PyList_GetItem(list, i)};
        std::string_view view;
        if (false == parse_PyString_as_string_view(wildcard, view)) {
            return false;
        }
        query.add_query(view);
    }
    return true;
}

extern "C" {
static auto PyQuery_new(PyTypeObject* type, PyObject* args, PyObject* keywords) -> PyObject* {
    PyQuery* self{reinterpret_cast<PyQuery*>(type->tp_alloc(type, 0))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        Py_RETURN_NONE;
    }
    self->query = nullptr;
    return reinterpret_cast<PyObject*>(self);
}

static auto PyQuery_init(PyQuery* self, PyObject* args, PyObject* keywords) -> int {
    /**
     __init__(query_list=None, case_sensitive=True, ts_lower_bound=0, ts_upper_bound=INT_MAX)
     */
    static char keyword_query_list[] = "query_list";
    static char keyword_case_sensitive[] = "case_sensitive";
    static char keyword_ts_lower_bound[] = "ts_lower_bound";
    static char keyword_ts_upper_bound[] = "ts_upper_bound";
    static char* keyword_table[] = {
            static_cast<char*>(keyword_query_list),
            static_cast<char*>(keyword_case_sensitive),
            static_cast<char*>(keyword_ts_lower_bound),
            static_cast<char*>(keyword_ts_upper_bound),
            nullptr};

    assert(nullptr == self->query);
    int py_case_sensitive{1};
    ffi::epoch_time_ms_t ts_lower_bound{Query::cDefaultTimestampLowerBound};
    ffi::epoch_time_ms_t ts_upper_bound{Query::cDefaultTimestampUpperBound};
    PyObject* py_query_list{Py_None};

    if (false == PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "|OpLL",
                         keyword_table,
                         &py_query_list,
                         &py_case_sensitive,
                         &ts_lower_bound,
                         &ts_upper_bound)) {
        return -1;
    }

    if (Py_None != py_query_list && false == PyObject_TypeCheck(py_query_list, &PyList_Type)) {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
        return -1;
    }

    bool const case_sensitive{(1 == py_case_sensitive) ? true : false};

    self->query = new Query(case_sensitive, ts_lower_bound, ts_upper_bound);
    if (Py_None == py_query_list) {
        return 0;
    }

    if (false == deserialize_query_list(*(self->query), py_query_list)) {
        return -1;
    }

    return 0;
}

static void PyQuery_dealloc(PyQuery* self) {
    delete self->query;
    PyObject_Del(self);
}

static auto PyQuery_match(PyQuery* self, PyObject* args) -> PyObject* {
    PyObject* message_obj;
    if (false == PyArg_ParseTuple(args, "O", &message_obj)) {
        return nullptr;
    }
    Message* message{nullptr};
    try {
        message = reinterpret_cast<PyMessage*>(message_obj)->message;
    } catch (std::exception const& ex) {
        PyErr_SetString(PyExc_TypeError, ex.what());
        return nullptr;
    }
    assert(message);
    bool const match_result{self->query->matches(*message)};
    if (match_result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static auto PyQuery_set_ts_lower_bound(PyQuery* self, PyObject* args) -> PyObject* {
    ffi::epoch_time_ms_t ts_lower_bound;
    assert(self->query);
    if (false == PyArg_ParseTuple(args, "L", ts_lower_bound)) {
        return nullptr;
    }
    self->query->set_ts_lower_bound(ts_lower_bound);
    return reinterpret_cast<PyObject*>(self);
}

static auto PyQuery_set_ts_upper_bound(PyQuery* self, PyObject* args) -> PyObject* {
    ffi::epoch_time_ms_t ts_upper_bound;
    assert(self->query);
    if (false == PyArg_ParseTuple(args, "L", ts_upper_bound)) {
        return nullptr;
    }
    self->query->set_ts_upper_bound(ts_upper_bound);
    return reinterpret_cast<PyObject*>(self);
}

static constexpr char cStateQueryList[] = "query_list";
static constexpr char cStateCaseSensitive[] = "case_sensitive";
static constexpr char cStateTsUpperBound[] = "ts_upper_bound";
static constexpr char cStateTsLowerBound[] = "ts_lower_bound";

static auto PyQuery___getstate__(PyQuery* self) -> PyObject* {
    assert(self->query);
    auto query_list{serialize_query_list(*(self->query))};
    if (nullptr == query_list) {
        return nullptr;
    }
    auto value = Py_BuildValue(
            "{sOsOsLsL}",
            cStateQueryList,
            query_list,
            cStateCaseSensitive,
            self->query->is_case_sensitive() ? Py_True : Py_False,
            cStateTsUpperBound,
            self->query->get_ts_upper_bound(),
            cStateTsLowerBound,
            self->query->get_ts_lower_bound());
    return value;
}

static auto PyQuery___setstate__(PyQuery* self, PyObject* state) -> PyObject* {
    if (false == PyDict_CheckExact(state)) {
        PyErr_SetString(PyExc_ValueError, clp_ffi_py::error_messages::pickled_state_error);
        return nullptr;
    }

    auto ts_upper_bound_obj{PyDict_GetItemString(state, cStateTsUpperBound)};
    if (nullptr == ts_upper_bound_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateTsUpperBound);
        return nullptr;
    }
    ffi::epoch_time_ms_t ts_upper_bound;
    if (false == parse_PyInt<ffi::epoch_time_ms_t>(ts_upper_bound_obj, ts_upper_bound)) {
        return nullptr;
    }

    auto ts_lower_bound_obj{PyDict_GetItemString(state, cStateTsLowerBound)};
    if (nullptr == ts_lower_bound_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateTsLowerBound);
        return nullptr;
    }
    ffi::epoch_time_ms_t ts_lower_bound;
    if (false == parse_PyInt<ffi::epoch_time_ms_t>(ts_lower_bound_obj, ts_lower_bound)) {
        return nullptr;
    }

    auto case_sensitive_obj{PyDict_GetItemString(state, cStateCaseSensitive)};
    if (nullptr == case_sensitive_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateCaseSensitive);
        return nullptr;
    }
    int is_case_sensitive{PyObject_IsTrue(case_sensitive_obj)};
    if (-1 == is_case_sensitive && PyErr_Occurred()) {
        return nullptr;
    }

    self->query = new Query(is_case_sensitive ? true : false, ts_lower_bound, ts_upper_bound);
    if (nullptr == self->query) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }

    auto query_list{PyDict_GetItemString(state, cStateQueryList)};
    if (nullptr == query_list) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::pickled_key_error_template,
                cStateQueryList);
        return nullptr;
    }
    if (false == deserialize_query_list(*(self->query), query_list)) {
        return nullptr;
    }

    Py_RETURN_NONE;
}
}

static PyMethodDef PyQuery_method_table[]{
        {"match",
         reinterpret_cast<PyCFunction>(PyQuery_match),
         METH_VARARGS,
         "Match the query with a given message."},
        {"set_ts_lower_bound",
         reinterpret_cast<PyCFunction>(PyQuery_set_ts_lower_bound),
         METH_VARARGS,
         "Set query lower bound timestamp"},
        {"set_ts_upper_bound",
         reinterpret_cast<PyCFunction>(PyQuery_set_ts_upper_bound),
         METH_VARARGS,
         "Set query upper bound timestamp"},
        {"__getstate__",
         reinterpret_cast<PyCFunction>(PyQuery___getstate__),
         METH_NOARGS,
         "Pickle the Query object"},
        {"__setstate__",
         reinterpret_cast<PyCFunction>(PyQuery___setstate__),
         METH_O,
         "Un-pickle the Query object"},
        {nullptr}};

static PyType_Slot PyQuery_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyQuery_dealloc)},
        {Py_tp_methods, PyQuery_method_table},
        {Py_tp_init, reinterpret_cast<void*>(PyQuery_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyQuery_new)},
        {0, nullptr}};

static PyType_Spec PyQuery_type_spec{
        "clp_ffi_py.CLPIRDecoder.Query",
        sizeof(PyQuery),
        0,
        Py_TPFLAGS_DEFAULT,
        PyQuery_slots};

static std::unique_ptr<PyTypeObject, PyObjectDeleter<PyTypeObject>> PyQuery_type;

auto PyQuery_get_PyType() -> PyTypeObject* {
    return PyQuery_type.get();
}

auto PyQuery_module_level_init(PyObject* py_module, std::vector<PyObject*>& object_list) -> bool {
    auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyQuery_type_spec))};
    PyQuery_type.reset(type);
    return add_type(
            reinterpret_cast<PyObject*>(PyQuery_get_PyType()),
            "Query",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
