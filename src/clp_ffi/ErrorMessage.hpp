#ifndef CLP_PY_ERROR_MESSAGE
#define CLP_PY_ERROR_MESSAGE

namespace clp_ffi_py::ErrorMessage {
constexpr char arg_parsing_error[] = "Failed to parse Python arguments.";
constexpr char arg_nullptr_error[] = "Arguments received are nullptr.";
constexpr char out_of_memory_error[] = "Failed to allocate memory.";
constexpr char module_loading_error[] = "Failed to load module: ";
constexpr char module_import_error[] = "Failed to import module: ";
constexpr char object_loading_error[] = "Failed to load C extension object: ";
constexpr char return_error[] = "Python method doesn't properly return.";
constexpr char not_implemented_error[] = "Feature not implemented.";
constexpr char capsule_fail_to_load_error[] = "Failed to load the capsule.";
} // namespace clp_ffi_py::ErrorMessage

namespace Encoding {
    constexpr char const* cTimestampError =
            "Native encoder cannot encode the given timestamp delta";
    constexpr char const* cPreambleError = "Native encoder cannot encode the given preamble";
    constexpr char const* cMessageError = "Native encoder cannot encode the given message";
} // namespace Encoding

namespace clp_ffi_py::ErrorMessage::Decoding {
constexpr char istream_empty_error[] = "Failed to read from istream when more bytes are expected.";
constexpr char ir_error_code[] = "IRErrorCode: ";
constexpr char invalid_metadata[] = "The encoded metadata is invalid.";
} // namespace clp_ffi_py::ErrorMessage::Decoding
#endif
