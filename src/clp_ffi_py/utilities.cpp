#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <iostream>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/utilities.hpp>

void clean_object_list(std::vector<PyObject*>& object_list) {
    for (auto type : object_list) {
        Py_DECREF(type);
    }
}

bool add_type(
        PyObject* new_type,
        char const* type_name,
        PyObject* module,
        std::vector<PyObject*>& object_list) {
    if (nullptr == new_type) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return false;
    }
    if (PyModule_AddObject(module, type_name, new_type) < 0) {
        std::string error_message{
                std::string(clp_ffi_py::error_messages::object_adding_error) +
                std::string(type_name)};
        PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
        return false;
    }
    return true;
}

bool add_capsule(
        void* ptr,
        char const* name,
        PyCapsule_Destructor destructor,
        PyObject* module,
        std::vector<PyObject*>& object_list) {
    PyObject* new_capsule{PyCapsule_New(ptr, name, destructor)};
    if (nullptr == new_capsule) {
        return false;
    }
    object_list.push_back(new_capsule);
    if (PyModule_AddObject(module, name, new_capsule) < 0) {
        return false;
    }
    return true;
}

void* get_capsule(PyObject* module, char const* key) {
    auto capsule{PyObject_GetAttrString(module, key)};
    void* retval{PyCapsule_GetPointer(capsule, key)};
    return retval;
}

auto parse_PyString(PyObject* Py_string, std::string& out) -> bool {
    if (false == PyUnicode_Check(Py_string)) {
        PyErr_SetString(PyExc_TypeError, "parse_PyString receives none-string argument.");
        return false;
    }

    auto str{PyUnicode_AsUTF8(Py_string)};
    if (nullptr == str) {
        return false;
    }
    out = std::string(str);
    return true;
}

auto parse_PyString_as_string_view(PyObject* Py_string, std::string_view& view) -> bool {
    if (false == PyUnicode_Check(Py_string)) {
        PyErr_SetString(PyExc_TypeError, "parse_PyString receives none-string argument.");
        return false;
    }

    auto str{PyUnicode_AsUTF8(Py_string)};
    if (nullptr == str) {
        return false;
    }
    view = std::string_view(str);
    return true;
}

void debug_message(std::string const& msg) {
    std::cerr << msg << "\n";
}
