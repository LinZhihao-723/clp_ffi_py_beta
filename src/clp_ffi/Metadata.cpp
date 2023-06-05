#include "Metadata.hpp"
#include "ErrorMessage.hpp"
#include "ExceptionFFI.hpp"

#include "../clp/components/core/src/ffi/ir_stream/protocol_constants.hpp"

static inline bool is_valid_json_string_data (nlohmann::json const& data, char const* key) {
    return data.contains(key) && data[key].is_string();
};

namespace clp_ffi_py::components {
Metadata::Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding) {
    if (false == is_four_byte_encoding) {
        throw ExceptionFFI(ErrorCode_Unsupported,
                           __FILE__,
                           __LINE__,
                           "Eight Byte Preamble is not yet supported.");
    }
    m_is_four_byte_encoding = is_four_byte_encoding;

    // Read ref_timestamp
    auto const ref_timestamp_key{ffi::ir_stream::cProtocol::Metadata::ReferenceTimestampKey};
    if (false == is_valid_json_string_data(metadata, ref_timestamp_key)) {
        throw ExceptionFFI(ErrorCode_MetadataCorrupted,
                           __FILE__,
                           __LINE__,
                           "Reference Timestamp cannot be found in the metadata.");
    }

    try {
        const std::string ref_timestamp_str{metadata[ref_timestamp_key]};
        m_ref_timestamp = std::stoull(ref_timestamp_str);
    } catch (std::exception const& ex) {
        // Handle the exception triggered by stoull
        throw ExceptionFFI(ErrorCode_Unsupported, __FILE__, __LINE__, ex.what());
    }

    // Read timestamp format
    auto const timestamp_format_key{ffi::ir_stream::cProtocol::Metadata::TimestampPatternKey};
    if (false == is_valid_json_string_data(metadata, timestamp_format_key)) {
        throw ExceptionFFI(ErrorCode_MetadataCorrupted,
                           __FILE__,
                           __LINE__,
                           "Timestamp Format cannot be found in the metadata.");
    }
    m_timestamp_format = metadata[timestamp_format_key];

    // Read timezone
    auto const timezone_key{ffi::ir_stream::cProtocol::Metadata::TimeZoneIdKey};
    if (false == is_valid_json_string_data(metadata, timezone_key)) {
        throw ExceptionFFI(ErrorCode_MetadataCorrupted,
                           __FILE__,
                           __LINE__,
                           "TimezoneID cannot be found in the metadata.");
    }
    m_timezone = metadata[timezone_key];
}
} // namespace clp_ffi_py::components
