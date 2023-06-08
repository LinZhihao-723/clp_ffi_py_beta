#ifndef CLP_PY_MESSAGE
#define CLP_PY_MESSAGE

#include "../../clp/components/core/src/ffi/encoding_methods.hpp"
#include "../../clp/components/core/src/string_utils.hpp"

#include "Metadata.hpp"

namespace clp_ffi_py::components {
class Message {
public:
    explicit Message() = default;

    std::string& get_message_ref () { return m_message; }
    ffi::epoch_time_ms_t& get_timestamp_ref () { return m_timestamp; }
    void set_timestamp (ffi::epoch_time_ms_t timestamp) { m_timestamp = timestamp; }
    bool wildcard_match (std::string_view wildcard) {
        return wildcard_match_unsafe(m_message, wildcard, false);
    }
    bool wildcard_match_case_sensitive (std::string_view wildcard) {
        return wildcard_match_unsafe(m_message, wildcard, true);
    }

private:
    std::string m_message;
    ffi::epoch_time_ms_t m_timestamp;
};
} // namespace clp_ffi_py::components

#endif
