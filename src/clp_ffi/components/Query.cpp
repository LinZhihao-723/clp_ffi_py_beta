#include "Query.hpp"

namespace clp_ffi_py::components {
bool Query::matches(Message const& message) const {
    return m_use_and ? matches_and(message) : matches_or(message);
}

bool Query::matches_and(Message const& message) const {
    for (auto const& query : m_query_list) {
        if (false == message.wildcard_match(query, m_case_sensitive)) {
            return false;
        }
    }
    return true;
}

bool Query::matches_or(Message const& message) const {
    for (auto const& query : m_query_list) {
        if (message.wildcard_match(query, m_case_sensitive)) {
            return true;
        }
    }
    return false;
}
} // namespace clp_ffi_py::components
