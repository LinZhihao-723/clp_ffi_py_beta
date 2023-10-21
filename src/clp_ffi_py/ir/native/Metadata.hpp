#ifndef CLP_FFI_PY_METADATA_HPP
#define CLP_FFI_PY_METADATA_HPP

#include <unordered_map>
#include <utility>
#include <vector>

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <clp/components/core/src/ffi/ir_stream/attributes.hpp>
#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A class that represents a decoded IR preamble. Contains ways to access (get)
 * metadata such as the timestamp format. After construction, the metadata is
 * readonly. */
class Metadata {
public:
    /**
     * Constructs a new Metadata object by reading values from a JSON object
     * decoded from the preamble. This constructor will validate the JSON data
     * and throw exceptions when failing to extract required values.
     * @param metadata JSON data that contains the metadata.
     * @param is_four_byte_encoding
     */
    explicit Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding);

    /**
     * Constructs a new Metadata object from the provided fields. Currently,
     * `m_is_four_byte_encoding` is set to true by default since it is the only
     * format supported.
     * @param ref_timestamp The reference timestamp used to calculate the
     * timestamp of the first log message in the IR stream.
     * @param timestamp_format Timestamp format to use when generating the logs
     * with a reader.
     * @param timezone Timezone in TZID format to use when generating the
     * timestamp from Unix epoch time.
     */
    explicit Metadata(
            ffi::epoch_time_ms_t ref_timestamp,
            std::string timestamp_format,
            std::string timezone
    )
            : m_is_four_byte_encoding{true},
              m_ref_timestamp{ref_timestamp},
              m_timestamp_format{std::move(timestamp_format)},
              m_timezone_id{std::move(timezone)} {};

    [[nodiscard]] auto is_using_four_byte_encoding() const -> bool {
        return m_is_four_byte_encoding;
    }

    [[nodiscard]] auto get_ref_timestamp() const -> ffi::epoch_time_ms_t { return m_ref_timestamp; }

    [[nodiscard]] auto get_timestamp_format() const -> std::string const& {
        return m_timestamp_format;
    }

    [[nodiscard]] auto get_timezone_id() const -> std::string const& { return m_timezone_id; }

    [[nodiscard]] auto get_num_attributes() const -> size_t { return m_attribute_table.size(); }

    [[nodiscard]] auto get_attribute_table() const
            -> std::vector<ffi::ir_stream::AttributeInfo> const& {
        return m_attribute_table;
    }

    [[nodiscard]] auto get_attribute_idx_map() const
            -> std::unordered_map<std::string, size_t> const& {
        return m_attribute_idx_map;
    }

    [[nodiscard]] auto get_attribute_idx(std::string const& attr_name) const -> size_t;

    [[nodiscard]] auto is_android_log() const -> bool {
        return m_android_build_version.has_value();
    }

private:
    bool m_is_four_byte_encoding;
    ffi::epoch_time_ms_t m_ref_timestamp;
    std::string m_timestamp_format;
    std::string m_timezone_id;
    std::vector<ffi::ir_stream::AttributeInfo> m_attribute_table;
    std::unordered_map<std::string, size_t> m_attribute_idx_map;
    std::optional<std::string> m_android_build_version;
};
}  // namespace clp_ffi_py::ir::native
#endif  // CLP_FFI_PY_METADATA_HPP
