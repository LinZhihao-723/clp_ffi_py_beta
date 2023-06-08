#ifndef CLP_FFI_PY_Query
#define CLP_FFI_PY_Query

#include "Message.hpp"
#include <vector>

namespace clp_ffi_py::components {
class Query {
public:
    Query(bool use_and, bool case_sensitive)
        : m_use_and(use_and),
          m_case_sensitive(case_sensitive){};
    void add_query (std::string_view wildcard) { m_query_list.emplace_back(wildcard); }
    [[nodiscard]] bool matches (Message const& message) const;

private:
    [[nodiscard]] bool matches_and (Message const& message) const;
    [[nodiscard]] bool matches_or (Message const& message) const;
    std::vector<std::string> m_query_list;
    bool const m_use_and;
    bool const m_case_sensitive;
};
} // namespace clp_ffi_py::components
#endif
