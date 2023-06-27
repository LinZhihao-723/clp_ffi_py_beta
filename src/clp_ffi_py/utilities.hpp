#ifndef CLP_FFI_PY_UTILITIES
#define CLP_FFI_PY_UTILITIES

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

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

auto parse_PyString(PyObject* Py_string, std::string& out) -> bool;

auto parse_PyString_as_string_view(PyObject* Py_string, std::string_view& view) -> bool;

template <typename int_type>
auto parse_PyInt(PyObject* Py_int, int_type& val) -> bool;

typedef bool (*TypeInitFunc)(PyObject*, std::vector<PyObject*>&);

#include <clp_ffi_py/utilities.tpp>

#endif
