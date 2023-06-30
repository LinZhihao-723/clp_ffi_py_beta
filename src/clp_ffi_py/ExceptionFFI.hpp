#ifndef CLP_FFI_PY_EXCEPTION_FFI
#define CLP_FFI_PY_EXCEPTION_FFI

#include <clp/components/core/src/TraceableException.hpp>

#include <string>

namespace clp_ffi_py {
class ExceptionFFI : public TraceableException {
public:
    ExceptionFFI(
            ErrorCode error_code,
            char const* const filename,
            int line_number,
            std::string message)
        : TraceableException(error_code, filename, line_number),
          m_message(std::move(message)) {}

    [[nodiscard]] char const* what() const noexcept override { return m_message.c_str(); }

private:
    std::string m_message;
};
} // namespace clp_ffi_py

#endif
