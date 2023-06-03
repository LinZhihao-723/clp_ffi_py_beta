#include "Metadata.hpp"

static PyMethodDef PyMetadata_method_table[] = {
        {"is_using_four_byte_encoding",
         (PyCFunction)clp_ffi_py::metadata::PyMetadata_is_using_four_byte_encoding,
         METH_NOARGS,
         "Check the encoding method (either 4byte or 8byte) from the metadata."},

        {"get_ref_timestamp",
         (PyCFunction)clp_ffi_py::metadata::PyMetadata_get_ref_timestamp,
         METH_NOARGS,
         "Get reference timestamp as an integer."},

        {"get_timestamp_format",
         (PyCFunction)clp_ffi_py::metadata::PyMetadata_get_timestamp_format,
         METH_NOARGS,
         "Get timestamp format as a string."},

        {"get_timezone",
         (PyCFunction)clp_ffi_py::metadata::PyMetadata_get_timezone,
         METH_NOARGS,
         "Get timezone as a string."},

        {NULL}};

static PyType_Slot PyMetadata_slots[] = {
        {Py_tp_dealloc, (void*)clp_ffi_py::metadata::PyMetadata_dealloc},
        {Py_tp_methods, PyMetadata_method_table},
        {Py_tp_init, (void*)clp_ffi_py::metadata::PyMetadata_init},
        {Py_tp_new, (void*)clp_ffi_py::metadata::PyMetadata_new},
        {0, NULL} // sentinel
};

PyType_Spec clp_ffi_py::metadata::PyMetadataTy = {
        "IR.Metadata", sizeof(PyMetadata), 0, Py_TPFLAGS_DEFAULT, PyMetadata_slots};

static struct PyModuleDef ir_module = {PyModuleDef_HEAD_INIT, "IR", "CLP IR", -1, NULL};

// in the module initialization function
PyMODINIT_FUNC PyInit_IR (void) {
    PyObject* new_module = PyModule_Create(&ir_module);
    if (nullptr == new_module) {
        return nullptr;
    }

    PyObject* PyMetadataType = PyType_FromSpec(&clp_ffi_py::metadata::PyMetadataTy);
    if (nullptr == PyMetadataType) {
        return nullptr;
    }

    Py_INCREF(PyMetadataType);
    if (PyModule_AddObject(new_module, "Metadata", PyMetadataType) < 0) {
        Py_DECREF(PyMetadataType);
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
