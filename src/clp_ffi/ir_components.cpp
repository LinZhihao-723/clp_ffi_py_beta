#include "ErrorMessage.hpp"
#include "Message.hpp"
#include "Metadata.hpp"

#include <iostream>
#include <utility>
#include <vector>

namespace {
void inline clean_type_list(std::vector<PyObject*>& type_list) {
    for (auto type : type_list) {
        Py_DECREF(type);
    }
}

bool inline add_type(PyType_Spec* py_type,
                     char const* type_name,
                     PyObject* module,
                     std::vector<PyObject*>& type_list) {
    PyObject* new_type{PyType_FromSpec(py_type)};
    if (nullptr == new_type) {
        return false;
    }
    type_list.push_back(new_type);
    Py_INCREF(new_type);
    if (PyModule_AddObject(module, type_name, new_type) < 0) {
        return false;
    }
    return true;
}
} // namespace

static struct PyModuleDef ir_module = {
        PyModuleDef_HEAD_INIT, "IRComponents", "CLP IR Components", -1, NULL};

static std::vector<std::pair<PyType_Spec*, char const*>> type_table{
        {&clp_ffi_py::metadata::PyMetadataTy, "Metadata"},
        {&clp_ffi_py::message::PyMessageTy, "Message"}};

// in the module initialization function
PyMODINIT_FUNC PyInit_IRComponents (void) {
    PyObject* new_module{PyModule_Create(&ir_module)};
    if (nullptr == new_module) {
        std::string error_message = std::string(clp_ffi_py::ErrorMessage::module_loading_error) +
                                    std::string(ir_module.m_name);
        PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
        return nullptr;
    }

    std::vector<PyObject*> type_list;
    for (auto [type, type_name] : type_table) {
        if (false == add_type(type, type_name, new_module, type_list)) {
            clean_type_list(type_list);
            Py_DECREF(new_module);
            std::string error_message = std::string(clp_ffi_py::ErrorMessage::type_loading_error) +
                                        std::string(type_name);
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }

    return new_module;
}
