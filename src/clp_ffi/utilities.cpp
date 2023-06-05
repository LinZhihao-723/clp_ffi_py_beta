#include <Python.h>
#include <iostream>

#include "utilities.hpp"

void clean_object_list (std::vector<PyObject*>& object_list) {
    for (auto type : object_list) {
        Py_DECREF(type);
    }
}

bool add_type (PyObject* new_type,
               char const* type_name,
               PyObject* module,
               std::vector<PyObject*>& object_list) {
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

bool add_capsule (void* ptr,
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

void* get_capsule (PyObject* module, char const* key) {
    auto capsule{PyObject_GetAttrString(module, key)};
    void* retval{PyCapsule_GetPointer(capsule, key)};
    return retval;
}
