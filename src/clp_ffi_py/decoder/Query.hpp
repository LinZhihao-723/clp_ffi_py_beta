#ifndef CLP_FFI_PY_Query
#define CLP_FFI_PY_Query

#include <clp_ffi_py/decoder/Message.hpp>
#include <vector>

namespace clp_ffi_py::decoder {
class Query {
public:
    Query(bool use_and, bool case_sensitive)
        : m_use_and(use_and),
          m_case_sensitive(case_sensitive){};
    void add_query(std::string_view wildcard) { m_query_list.emplace_back(wildcard); }
    [[nodiscard]] auto matches(Message const& message) const -> bool;

private:
    [[nodiscard]] auto matches_and(Message const& message) const -> bool;
    [[nodiscard]] auto matches_or(Message const& message) const -> bool;
    std::vector<std::string> m_query_list;
    bool const m_use_and;
    bool const m_case_sensitive;
};
} // namespace clp_ffi_py::decoder
#endif
