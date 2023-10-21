
#ifndef CLP_FFI_PY_IR_UTILS_HPP
#define CLP_FFI_PY_IR_UTILS_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/ir/native/LogEvent.hpp>

namespace clp_ffi_py::ir::native {
/**
 * Serializes the attributes from a C++ attribute table into the Python dict.
 * @param attributes
 * @return Python dict with the serialized [name, value] attribute pairs on
 * success.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto serialize_attributes_to_python_dict(LogEvent::attribute_table_t const& attributes)
        -> PyObject*;

/**
 * Deserializes the attributes from the Python dict.
 * @param py_attr_dict
 * @param attributes
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto deserialize_attributes_from_python_dict(
        PyObject* py_attr_dict,
        LogEvent::attribute_table_t& attributes
) -> bool;
}  // namespace clp_ffi_py::ir::native

#endif
