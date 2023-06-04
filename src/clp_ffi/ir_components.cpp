#include "ErrorMessage.hpp"
#include "PyMessage.hpp"
#include "PyMetadata.hpp"

#include "PyDecoder.hpp"

#include <vector>

namespace {
void inline clean_object_list(std::vector<PyObject*>& object_list) {
    for (auto type : object_list) {
        Py_DECREF(type);
    }
}

bool inline add_type(PyType_Spec* py_type,
                     char const* type_name,
                     PyObject* module,
                     std::vector<PyObject*>& object_list) {
    PyObject* new_type{PyType_FromSpec(py_type)};
    if (nullptr == new_type) {
        return false;
    }
    object_list.push_back(new_type);
    Py_INCREF(new_type);
    if (PyModule_AddObject(module, type_name, new_type) < 0) {
        return false;
    }
    return true;
}

bool inline add_capsule(void* ptr,
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
} // namespace

static struct PyModuleDef ir_module = {
        PyModuleDef_HEAD_INIT, "IRComponents", "CLP IR Components", -1, NULL};

static std::vector<std::pair<PyType_Spec*, char const*>> type_table{
        {&clp_ffi_py::components::PyMetadataTy, "Metadata"},
        {&clp_ffi_py::components::PyMessageTy, "Message"},
        {&clp_ffi_py::decoder::PyDecoderBufferTy, "DecoderBuffer"}};

static std::vector<std::pair<void*, char const*>> api_table{
        {reinterpret_cast<void*>(&clp_ffi_py::components::PyMetadata_init_from_json),
         clp_ffi_py::components::PyMetadata_init_from_json_key},
        {reinterpret_cast<void*>(&clp_ffi_py::components::PyMessage_create_empty),
         clp_ffi_py::components::PyMessage_create_empty_key}};

// in the module initialization function
PyMODINIT_FUNC PyInit_IRComponents (void) {
    std::vector<PyObject*> object_list;
    PyObject* new_module{PyModule_Create(&ir_module)};
    if (nullptr == new_module) {
        std::string error_message{std::string(clp_ffi_py::ErrorMessage::module_loading_error) +
                                  std::string(ir_module.m_name)};
        PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
        return nullptr;
    }
    object_list.push_back(new_module);

    for (auto [type, type_name] : type_table) {
        if (false == add_type(type, type_name, new_module, object_list)) {
            clean_object_list(object_list);
            std::string error_message{std::string(clp_ffi_py::ErrorMessage::object_loading_error) +
                                      std::string(type_name)};
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }

    for (auto [api, name] : api_table) {
        if (false == add_capsule(api, name, nullptr, new_module, object_list)) {
            clean_object_list(object_list);
            std::string error_message{std::string(clp_ffi_py::ErrorMessage::object_loading_error) +
                                      std::string(name)};
            PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
            return nullptr;
        }
    }

    return new_module;
}
