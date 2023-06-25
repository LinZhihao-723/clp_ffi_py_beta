#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/PyQuery.hpp>

#include <clp/components/core/src/ffi/encoding_methods.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectDeleter.hpp>
#include <clp_ffi_py/decoder/PyMessage.hpp>
#include <clp_ffi_py/utilities.hpp>

namespace clp_ffi_py::decoder {
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
    if (nullptr == self->query) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return -1;
    }

    if (Py_None == py_query_list) {
        return 0;
    }

    // Note: we don't have to deallocate self->query because dealloc function
    // will handle memory management
    Py_ssize_t const list_size{PyList_Size(py_query_list)};
    for (Py_ssize_t i{0}; i < list_size; ++i) {
        PyObject* item{PyList_GetItem(py_query_list, i)};
        if (!PyUnicode_Check(item)) {
            PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
            return -1;
        }

        char const* wildcard{PyUnicode_AsUTF8(item)};
        if (nullptr == wildcard) {
            // PyUnicode_AsUTF8 sets the exception
            return -1;
        }

        self->query->add_query(std::string_view(wildcard));
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
        {nullptr}};

static PyType_Slot PyQuery_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyQuery_dealloc)},
        {Py_tp_methods, PyQuery_method_table},
        {Py_tp_init, reinterpret_cast<void*>(PyQuery_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyQuery_new)},
        {0, nullptr}};

static PyType_Spec PyQuery_type_spec{
        "CLPIRDecoder.Query",
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
    if (nullptr != type) {
        Py_INCREF(type);
    }
    return add_type(
            reinterpret_cast<PyObject*>(PyQuery_get_PyType()),
            "Query",
            py_module,
            object_list);
}
} // namespace clp_ffi_py::decoder
