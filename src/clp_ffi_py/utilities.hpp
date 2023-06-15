#ifndef CLP_FFI_PY_UTILITIES
#define CLP_FFI_PY_UTILITIES

#include "Python.hpp"
#include <iostream>
#include <string>
#include <vector>

void clean_object_list(std::vector<PyObject*>& object_list);

bool add_type(
        PyObject* new_type,
        char const* type_name,
        PyObject* module,
        std::vector<PyObject*>& object_list);

bool add_capsule(
        void* ptr,
        char const* name,
        PyCapsule_Destructor destructor,
        PyObject* module,
        std::vector<PyObject*>& object_list);

void* get_capsule(PyObject* module, char const* key);

void debug_message(std::string const& msg);

typedef bool (*TypeInitFunc)(PyObject*, std::vector<PyObject*>&);

#endif
