#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/decoding_methods.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/decoder/PyDecoderBuffer.hpp>
#include <clp_ffi_py/decoder/PyMessage.hpp>
#include <clp_ffi_py/decoder/PyMetadata.hpp>
#include <clp_ffi_py/decoder/PyQuery.hpp>
#include <clp_ffi_py/utilities.hpp>

static PyMethodDef DecoderMethods[] = {
        {"decode_preamble",
         clp_ffi_py::decoder::four_byte_decoder::decode_preamble,
         METH_VARARGS,
         "Decode a preamble and return a PyMetadata object."},
        {"decode_next_message",
         clp_ffi_py::decoder::four_byte_decoder::decode_next_message,
         METH_VARARGS,
         "Decode next message and return a PyMessage object."},
        {"decode_next_message_with_query",
         clp_ffi_py::decoder::four_byte_decoder::decode_next_message_with_query,
         METH_VARARGS,
         "Decode next message and return a PyMessage object that matches the given query."},
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef clp_four_byte_decoder =
        {PyModuleDef_HEAD_INIT, "CLPIRDecoder", NULL, -1, DecoderMethods};

PyMODINIT_FUNC PyInit_CLPIRDecoder() {
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
    if (false == clp_ffi_py::decoder::PyDecoderBuffer_module_level_init(new_module, object_list)) {
        clean_object_list(object_list);
        return nullptr;
    }

    if (false == clp_ffi_py::decoder::PyMetadata_module_level_init(new_module, object_list)) {
        clean_object_list(object_list);
        return nullptr;
    }

    if (false == clp_ffi_py::decoder::PyMessageTy_module_level_init(new_module, object_list)) {
        clean_object_list(object_list);
        return nullptr;
    }

    if (false == clp_ffi_py::decoder::PyQuery_module_level_init(new_module, object_list)) {
        clean_object_list(object_list);
        return nullptr;
    }

    return new_module;
}
