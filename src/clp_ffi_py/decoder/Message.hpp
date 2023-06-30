#ifndef CLP_FFI_PY_MESSAGE
#define CLP_FFI_PY_MESSAGE

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <clp/components/core/src/string_utils.hpp>

namespace clp_ffi_py::decoder {
class Message {
public:
    explicit Message() = default;

    explicit Message(
            std::string_view message,
            std::string_view formatted_timestamp,
            ffi::epoch_time_ms_t timestamp,
            size_t message_idx)
        : m_message{message},
          m_formatted_timestamp{formatted_timestamp},
          m_timestamp{timestamp},
          m_message_idx{message_idx} {};

    explicit Message(std::string_view message, ffi::epoch_time_ms_t timestamp, size_t message_idx)
        : m_message{message},
          m_timestamp{timestamp},
          m_message_idx{message_idx} {};

    [[nodiscard]] auto get_message() const -> std::string { return m_message; }

    [[nodiscard]] auto get_timestamp() const -> ffi::epoch_time_ms_t { return m_timestamp; }

    [[nodiscard]] auto get_formatted_timestamp() const -> std::string {
        return m_formatted_timestamp;
    }

    [[nodiscard]] auto get_message_idx() const -> size_t { return m_message_idx; }

    [[nodiscard]] auto get_message_view() const -> std::string_view {
        return std::string_view(m_message);
    }

    [[nodiscard]] auto has_formatted_timestamp() const -> bool {
        return (false == m_formatted_timestamp.empty());
    }

    void set_message(std::string_view message) { m_message = message; }

    void set_timestamp(ffi::epoch_time_ms_t timestamp) { m_timestamp = timestamp; }

    void set_formatted_timestamp(std::string const& formatted_timestamp) {
        m_formatted_timestamp = formatted_timestamp;
    }

    void set_message_idx(size_t message_idx) { m_message_idx = message_idx; }

    [[nodiscard]] auto
    wildcard_match(std::string const& wildcard, bool use_case_sensitive = false) const -> bool {
        return wildcard_match_unsafe(m_message, std::string_view(wildcard), use_case_sensitive);
    }

    [[nodiscard]] auto wildcard_match_case_sensitive(std::string const& wildcard) const -> bool {
        return wildcard_match_unsafe(m_message, std::string_view(wildcard), true);
    }

    [[nodiscard]] auto
    wildcard_match(std::string_view wildcard, bool use_case_sensitive = false) const -> bool {
        return wildcard_match_unsafe(m_message, wildcard, use_case_sensitive);
    }

    [[nodiscard]] auto wildcard_match_case_sensitive(std::string_view wildcard) const -> bool {
        return wildcard_match_unsafe(m_message, wildcard, true);
    }

private:
    std::string m_message;
    std::string m_formatted_timestamp;
    ffi::epoch_time_ms_t m_timestamp;
    size_t m_message_idx;
};
} // namespace clp_ffi_py::decoder

#endif
