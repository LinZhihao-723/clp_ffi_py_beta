#ifndef CLP_FFI_PY_ERROR_MESSAGE
#define CLP_FFI_PY_ERROR_MESSAGE

namespace clp_ffi_py::error_messages {
// Note: each every message is defined using pointer instead of an array to
// address clang-tidy warnings.
constexpr char arg_parsing_error[] = "Failed to parse Python arguments.";
constexpr char arg_nullptr_error[] = "Arguments received are nullptr.";
constexpr char out_of_memory_error[] = "Failed to allocate memory.";
constexpr char module_loading_error[] = "Failed to load module: ";
constexpr char module_import_error[] = "Failed to import module: ";
constexpr char object_adding_error[] = "Failed to add C extension object: ";
constexpr char return_error[] = "Python method doesn't properly return.";
constexpr char not_implemented_error[] = "Feature not implemented.";
constexpr char capsule_fail_to_load_error[] = "Failed to load the capsule.";
constexpr char py_type_error[] = "Wrong Py Type received.";
constexpr char pickled_state_error[] = "Pickled state object is not a dict.";
constexpr char pickled_key_error_template[] = "No \"%s\" in pickled dict.";

namespace encoder {
    constexpr char const* cTimestampError =
            "Native encoder cannot encode the given timestamp delta";
    constexpr char const* cPreambleError = "Native encoder cannot encode the given preamble";
    constexpr char const* cMessageError = "Native encoder cannot encode the given message";
} // namespace encoder

namespace decoder {
    constexpr char istream_empty_error[] =
            "Failed to read from istream when more bytes are expected.";
    constexpr char ir_error_code[] = "IRErrorCode: ";
    constexpr char invalid_metadata[] = "The encoded metadata is invalid.";
} // namespace decoder
} // namespace clp_ffi_py::error_messages
#endif
