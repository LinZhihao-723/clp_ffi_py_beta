#ifndef CLP_PY_ERROR_MESSAGE
#define CLP_PY_ERROR_MESSAGE

namespace clp_ffi_py::ErrorMessage {
constexpr char arg_parsing_error[] = "Native preamble encoder failed to parse Python arguments.";
constexpr char out_of_memory_error[] = "Failed to allocate memory.";
} // namespace clp_ffi_py::ErrorMessage

namespace Encoding {
    constexpr char const* cTimestampError =
            "Native encoder cannot encode the given timestamp delta";
    constexpr char const* cPreambleError = "Native encoder cannot encode the given preamble";
    constexpr char const* cMessageError = "Native encoder cannot encode the given message";
} // namespace Encoding

#endif
