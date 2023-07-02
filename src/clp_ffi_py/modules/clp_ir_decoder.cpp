#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/decoder/decoding_methods.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/decoder/PyDecoderBuffer.hpp>
#include <clp_ffi_py/decoder/PyMessage.hpp>
#include <clp_ffi_py/decoder/PyMetadata.hpp>
#include <clp_ffi_py/decoder/PyQuery.hpp>
#include <clp_ffi_py/utilities.hpp>

static PyMethodDef DecoderMethods[] = {
        {"decode_preamble",
         reinterpret_cast<PyCFunction>(clp_ffi_py::decoder::four_byte_decoder::decode_preamble),
         METH_VARARGS,
         "Decode a preamble and return a PyMetadata object."},
        {"decode_next_message",
         reinterpret_cast<PyCFunction>(clp_ffi_py::decoder::four_byte_decoder::decode_next_message),
         METH_VARARGS | METH_KEYWORDS,
         "Decode next message and return a PyMessage object."},
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef clp_ir_decoder =
        {PyModuleDef_HEAD_INIT, "CLPIRDecoder", nullptr, -1, DecoderMethods};

extern "C" {
PyMODINIT_FUNC PyInit_CLPIRDecoder() {
    // Create the module
    std::vector<PyObject*> object_list;
    PyObject* new_module{PyModule_Create(&clp_ir_decoder)};
    if (nullptr == new_module) {
        std::string error_message{
                std::string(clp_ffi_py::error_messages::module_loading_error) +
                std::string(clp_ir_decoder.m_name)};
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

    if (false == clp_ffi_py::Py_utils_init()) {
        clean_object_list(object_list);
        return nullptr;
    }

    return new_module;
}
}
