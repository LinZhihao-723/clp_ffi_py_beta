#include "../ErrorMessage.hpp"
#include "../Python.hpp"
#include "../decoder/PyDecoderBuffer.hpp"
#include "../decoder/decoding_methods.hpp"
#include "../utilities.hpp"

static PyMethodDef DecoderMethods[] = {
        {"decode_preamble",
         clp_ffi_py::decoder::four_byte_decoder::decode_preamble,
         METH_VARARGS,
         "Decode a preamble and return a PyMetadata object."},
        {"decode_next_message",
         clp_ffi_py::decoder::four_byte_decoder::decode_next_message,
         METH_VARARGS,
         "Decode next messaage and return a PyMessage object."},
        {"decode_next_message_with_query",
         clp_ffi_py::decoder::four_byte_decoder::decode_next_message_with_query,
         METH_VARARGS,
         "Decode next messaage and return a PyMessage object that matches the given query."},
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef clp_four_byte_decoder =
        {PyModuleDef_HEAD_INIT, "CLPFourByteDecoder", NULL, -1, DecoderMethods};

PyMODINIT_FUNC PyInit_CLPFourByteDecoder (void) {
    // Create the module
    std::vector<PyObject*> object_list;
    PyObject* new_module{PyModule_Create(&clp_four_byte_decoder)};
    if (nullptr == new_module) {
        std::string error_message{
                std::string(clp_ffi_py::error_messages::module_loading_error) +
                std::string(clp_four_byte_decoder.m_name)};
        PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
        return nullptr;
    }
    object_list.push_back(new_module);

    // Add the type
    auto new_type{clp_ffi_py::decoder::PyDecoderBuffer_get_PyType()};
    char const* type_name = "DecoderBuffer";
    if (false == add_type(new_type, type_name, new_module, object_list)) {
        clean_object_list(object_list);
        std::string error_message{
                std::string(clp_ffi_py::error_messages::object_loading_error) +
                std::string(type_name)};
        PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
        return nullptr;
    }

    // Import dependency modules and get capsules
    PyObject* ir_component_module{PyImport_ImportModule("clp_ffi_py.IRComponents")};
    if (nullptr == ir_component_module) {
        clean_object_list(object_list);
        std::string error_message{
                std::string(clp_ffi_py::error_messages::module_import_error) +
                std::string("clp_ffi_py.IRComponents")};
        PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
        return nullptr;
    }
    object_list.push_back(ir_component_module);

    void* retval;
    retval = get_capsule(ir_component_module, clp_ffi_py::components::PyMessage_create_empty_key);
    if (nullptr == retval) {
        clean_object_list(object_list);
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::capsule_fail_to_load_error);
        return nullptr;
    }
    clp_ffi_py::decoder::four_byte_decoder::PyMessage_create_empty =
            reinterpret_cast<clp_ffi_py::components::PyMessageCreateFuncType>(retval);

    retval =
            get_capsule(ir_component_module, clp_ffi_py::components::PyMetadata_init_from_json_key);
    if (nullptr == retval) {
        clean_object_list(object_list);
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::capsule_fail_to_load_error);
        return nullptr;
    }
    clp_ffi_py::decoder::four_byte_decoder::PyMetadata_init_from_json =
            reinterpret_cast<clp_ffi_py::components::PyMetadataCreateFuncType>(retval);

    return new_module;
}
