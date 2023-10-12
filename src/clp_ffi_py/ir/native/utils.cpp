#include "utils.hpp"

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
auto serialize_attributes_to_python_dict(LogEvent::attribute_table_t const& attributes)
        -> PyObject* {
    if (attributes.empty()) {
        Py_RETURN_NONE;
    }
    auto* py_attributes{PyDict_New()};
    if (nullptr == py_attributes) {
        return nullptr;
    }
    bool failed{false};
    for (auto const& [attr_name, attribute] : attributes) {
        PyObjectPtr<PyObject> const attr_name_py{PyUnicode_FromString(attr_name.c_str())};
        if (nullptr == attr_name_py.get()) {
            failed = true;
            break;
        }
        if (false == attribute.has_value()) {
            PyDict_SetItem(py_attributes, attr_name_py.get(), Py_None);
            continue;
        }
        auto const& attr_val{attribute.value()};
        PyObjectPtr<PyObject> attr_py{nullptr};
        if (attr_val.is_type<ffi::ir_stream::attr_int_t>()) {
            auto* attr_int_py{PyLong_FromLongLong(attr_val.get_value<ffi::ir_stream::attr_int_t>())
            };
            attr_py.reset(attr_int_py);
        } else if (attr_val.is_type<ffi::ir_stream::attr_str_t>()) {
            std::string_view attr_str{attr_val.get_value<ffi::ir_stream::attr_str_t>()};
            auto* attr_str_py{PyUnicode_FromString(attr_str.data())};
            attr_py.reset(attr_str_py);
        } else {
            PyErr_SetString(PyExc_NotImplementedError, "Unsupported attribute type");
        }
        if (nullptr == attr_py.get()) {
            failed = true;
            break;
        }
        if (-1 == PyDict_SetItem(py_attributes, attr_name_py.get(), attr_py.get())) {
            failed = true;
            break;
        }
    }
    if (failed) {
        Py_DECREF(py_attributes);
        return nullptr;
    }
    return py_attributes;
}

auto deserialize_attributes_from_python_dict(
        PyObject* py_attr_dict,
        LogEvent::attribute_table_t& attributes
) -> bool {
    attributes.clear();
    if (Py_None == py_attr_dict) {
        return true;
    }
    if (false == static_cast<bool>(PyDict_CheckExact(py_attr_dict))) {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::cPyTypeError);
        return false;
    }
    PyObject* py_attr_name{};
    PyObject* py_attr{};
    Py_ssize_t pos{0};
    ffi::ir_stream::attr_str_t attr_str;
    ffi::ir_stream::attr_int_t attr_int;

    std::string_view attr_name_view;
    while (static_cast<bool>(PyDict_Next(py_attr_dict, &pos, &py_attr_name, &py_attr))) {
        if (false == parse_py_string_as_string_view(py_attr_name, attr_name_view)) {
            PyErr_SetString(PyExc_TypeError, "String keys are expected in attribute table.");
            return false;
        }
        if (0 != attributes.count(attr_name_view.data())) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    "Repeated attribute key detected: %s",
                    attr_name_view.data()
            );
            return false;
        }
        if (Py_IsNone(py_attr)) {
            attributes.emplace(attr_name_view, std::nullopt);
            continue;
        }
        if (static_cast<bool>(PyUnicode_Check(py_attr))) {
            if (false == parse_py_string(py_attr, attr_str)) {
                return false;
            }
            attributes.emplace(attr_name_view, attr_str);
        } else if (static_cast<bool>(PyLong_Check(py_attr))) {
            if (false == parse_py_int<ffi::ir_stream::attr_int_t>(py_attr, attr_int)) {
                return false;
            }
            attributes.emplace(attr_name_view, attr_int);
        } else {
            PyErr_SetString(PyExc_TypeError, "Unknown serialized log attribute type.");
            return false;
        }
    }
    return true;
}
}  // namespace clp_ffi_py::ir::native
