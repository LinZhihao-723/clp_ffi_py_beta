#ifndef CLP_PY_Metadata
#define CLP_PY_Metadata

#include <Python.h>

#include "../clp/components/core/src/ffi/encoding_methods.hpp"
#include "../clp/components/core/submodules/json/single_include/nlohmann/json.hpp"

namespace clp_ffi_py::metadata {
class Metadata {
public:
    explicit Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding);

    explicit Metadata(ffi::epoch_time_ms_t ref_timestamp,
                      std::string const& timestamp_format,
                      std::string const& timezone)
        : m_is_four_byte_encoding(true),
          m_ref_timestamp(ref_timestamp),
          m_timestamp_format(timestamp_format),
          m_timezone(timezone) {}

    bool is_using_four_byte_encoding () const { return m_is_four_byte_encoding; }
    ffi::epoch_time_ms_t get_ref_timestamp () const { return m_ref_timestamp; }
    std::string const& get_timestamp_format () const { return m_timestamp_format; }
    std::string const& get_timezone () const { return m_timezone; }

private:
    bool m_is_four_byte_encoding;
    ffi::epoch_time_ms_t m_ref_timestamp;
    std::string m_timestamp_format;
    std::string m_timezone;
};

extern PyType_Spec PyMetadataTy;

struct PyMetadata {
    PyObject_HEAD;
    Metadata* metadata;
};

PyObject* PyMetadata_new (PyTypeObject* type, PyObject* args, PyObject* kwds);
int PyMetadata_init (PyMetadata* self, PyObject* args, PyObject* kwds);
void PyMetadata_dealloc (PyMetadata* self);
PyObject* PyMetadata_is_using_four_byte_encoding (PyMetadata* self);
PyObject* PyMetadata_get_ref_timestamp (PyMetadata* self);
PyObject* PyMetadata_get_timestamp_format (PyMetadata* self);
PyObject* PyMetadata_get_timezone (PyMetadata* self);

PyMetadata* PyMetadata_init_from_json (nlohmann::json const& metadata, bool is_four_byte_encoding);
} // namespace clp_ffi_py::metadata

#endif