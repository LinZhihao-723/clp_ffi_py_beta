#ifndef CLP_FFI_PY_METADATA
#define CLP_FFI_PY_METADATA

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>

namespace clp_ffi_py::decoder {
class Metadata {
public:
    explicit Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding);

    explicit Metadata(
            ffi::epoch_time_ms_t ref_timestamp,
            std::string const& timestamp_format,
            std::string const& timezone)
        : m_is_four_byte_encoding{true},
          m_ref_timestamp{ref_timestamp},
          m_timestamp_format{timestamp_format},
          m_timezone_id{timezone} {};

    [[nodiscard]] auto is_using_four_byte_encoding() const -> bool {
        return m_is_four_byte_encoding;
    }

    [[nodiscard]] auto get_ref_timestamp() const -> ffi::epoch_time_ms_t { return m_ref_timestamp; }

    [[nodiscard]] auto get_timestamp_format() const -> std::string const& {
        return m_timestamp_format;
    }

    [[nodiscard]] auto get_timezone_id() const -> std::string const& { return m_timezone_id; }

private:
    bool m_is_four_byte_encoding;
    ffi::epoch_time_ms_t m_ref_timestamp;
    std::string m_timestamp_format;
    std::string m_timezone_id;
};
} // namespace clp_ffi_py::decoder
#endif
