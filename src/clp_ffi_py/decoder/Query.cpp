#include <clp_ffi_py/decoder/Query.hpp>

#include <clp/components/core/src/string_utils.hpp>

namespace clp_ffi_py::decoder {
auto Query::matches(Message const& message) const -> bool {
    return matches(message.get_message_view());
}

auto Query::matches(std::string_view message) const -> bool {
    if (m_query_list.empty()) {
        return true;
    }
    return m_use_and ? matches_and(message) : matches_or(message);
}

auto Query::matches_and(std::string_view message) const -> bool {
    for (auto const& query : m_query_list) {
        if (false == wildcard_match_unsafe(message, query, m_case_sensitive)) {
            return false;
        }
    }
    return true;
}

auto Query::matches_or(std::string_view message) const -> bool {
    for (auto const& query : m_query_list) {
        if (wildcard_match_unsafe(message, query, m_case_sensitive)) {
            return true;
        }
    }
    return false;
}
} // namespace clp_ffi_py::decoder
