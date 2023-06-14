#ifndef CLP_FFI_PY_MESSAGE
#define CLP_FFI_PY_MESSAGE

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <clp/components/core/src/string_utils.hpp>

namespace clp_ffi_py::components {
class Message {
public:
    explicit Message() = default;

    std::string& get_message_ref () { return m_message; }
    ffi::epoch_time_ms_t& get_timestamp_ref () { return m_timestamp; }
    void set_timestamp (ffi::epoch_time_ms_t timestamp) { m_timestamp = timestamp; }
    [[nodiscard]] bool
    wildcard_match (std::string const& wildcard, bool use_case_sensitive = false) const {
        return wildcard_match_unsafe(m_message, std::string_view(wildcard), use_case_sensitive);
    }
    [[nodiscard]] bool wildcard_match_case_sensitive (std::string const& wildcard) const {
        return wildcard_match_unsafe(m_message, std::string_view(wildcard), true);
    }
    [[nodiscard]] bool
    wildcard_match (std::string_view wildcard, bool use_case_sensitive = false) const {
        return wildcard_match_unsafe(m_message, wildcard, use_case_sensitive);
    }
    [[nodiscard]] bool wildcard_match_case_sensitive (std::string_view wildcard) const {
        return wildcard_match_unsafe(m_message, wildcard, true);
    }

private:
    std::string m_message;
    ffi::epoch_time_ms_t m_timestamp;
};
} // namespace clp_ffi_py::components

#endif
