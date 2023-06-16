#include <clp_ffi_py/decoder/Query.hpp>

namespace clp_ffi_py::decoder {
auto Query::matches(Message const& message) const -> bool {
    if (m_query_list.empty()) {
        return true;
    }
    return m_use_and ? matches_and(message) : matches_or(message);
}

auto Query::matches_and(Message const& message) const -> bool {
    for (auto const& query : m_query_list) {
        if (false == message.wildcard_match(query, m_case_sensitive)) {
            return false;
        }
    }
    return true;
}

auto Query::matches_or(Message const& message) const -> bool {
    for (auto const& query : m_query_list) {
        if (message.wildcard_match(query, m_case_sensitive)) {
            return true;
        }
    }
    return false;
}
} // namespace clp_ffi_py::decoder
