#ifndef CLP_FFI_PY_LOG_EVENT_HPP
#define CLP_FFI_PY_LOG_EVENT_HPP

#include <optional>
#include <unordered_map>
#include <vector>

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <clp/components/core/src/ffi/ir_stream/attributes.hpp>
#include <gsl/span>

namespace clp_ffi_py::ir::native {
/**
 * A class that represents a decoded IR log event. Contains ways to access (get
 * or set) the log message, the timestamp, and the log event index.
 */
class LogEvent {
public:
    using attribute_table_t
            = std::unordered_map<std::string, std::optional<ffi::ir_stream::Attribute>>;

    LogEvent() = delete;

    /**
     * Constructs a new log event and leaves the formatted timestamp empty by
     * default.
     * @param log_message
     * @param timestamp
     * @param index
     * @param formatted_timestamp
     * @param encoded_log_event_view
     */
    explicit LogEvent(
            std::string_view log_message,
            ffi::epoch_time_ms_t timestamp,
            size_t index,
            attribute_table_t attributes,
            std::optional<std::string_view> formatted_timestamp = std::nullopt,
            std::optional<gsl::span<int8_t>> encoded_log_event_view = std::nullopt
    )
            : m_log_message{log_message},
              m_timestamp{timestamp},
              m_index{index},
              m_attributes(std::move(attributes)),
              m_cached_encoded_log_event_size{0} {
        if (formatted_timestamp.has_value()) {
            m_formatted_timestamp = std::string(formatted_timestamp.value());
        }
        if (encoded_log_event_view.has_value()) {
            m_cached_encoded_log_event_size = encoded_log_event_view->size();
            m_cached_encoded_log_event
                    = std::make_unique<int8_t[]>(m_cached_encoded_log_event_size);
            memcpy(m_cached_encoded_log_event.get(),
                   encoded_log_event_view->data(),
                   m_cached_encoded_log_event_size);
        }
    }

    [[nodiscard]] auto get_log_message() const -> std::string { return m_log_message; }

    [[nodiscard]] auto get_log_message_view() const -> std::string_view {
        return std::string_view{m_log_message};
    }

    [[nodiscard]] auto get_timestamp() const -> ffi::epoch_time_ms_t { return m_timestamp; }

    [[nodiscard]] auto get_formatted_timestamp() const -> std::string {
        return m_formatted_timestamp;
    }

    [[nodiscard]] auto get_index() const -> size_t { return m_index; }

    [[nodiscard]] auto get_attributes() const -> attribute_table_t const& { return m_attributes; }

    [[nodiscard]] auto has_attributes() const -> bool { return false == m_attributes.empty(); }

    [[nodiscard]] auto has_cached_encoded_log_event() const -> bool {
        return nullptr != m_cached_encoded_log_event.get();
    }

    [[nodiscard]] auto get_cached_encoded_log_event() const -> gsl::span<int8_t> {
        return gsl::span<int8_t>{m_cached_encoded_log_event.get(), m_cached_encoded_log_event_size};
    }

    /**
     * @return Whether the log event has the formatted timestamp buffered.
     */
    [[nodiscard]] auto has_formatted_timestamp() const -> bool {
        return (false == m_formatted_timestamp.empty());
    }

    auto set_log_message(std::string_view log_message) -> void { m_log_message = log_message; }

    auto set_timestamp(ffi::epoch_time_ms_t timestamp) -> void { m_timestamp = timestamp; }

    auto set_formatted_timestamp(std::string const& formatted_timestamp) -> void {
        m_formatted_timestamp = formatted_timestamp;
    }

    auto set_index(size_t index) -> void { m_index = index; }

private:
    std::string m_log_message;
    ffi::epoch_time_ms_t m_timestamp;
    size_t m_index;
    std::string m_formatted_timestamp;
    attribute_table_t m_attributes;
    std::unique_ptr<int8_t[]> m_cached_encoded_log_event;
    size_t m_cached_encoded_log_event_size;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_LOG_EVENT_HPP
