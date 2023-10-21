#include "Query.hpp"

#include <algorithm>
#include <string_view>

#include <clp/components/core/src/string_utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
/**
 * @param lhs
 * @param rhs
 * @return Whether two attributes are the same.
 */
auto compare_attr_val(
        std::optional<ffi::ir_stream::Attribute> const& lhs,
        std::optional<ffi::ir_stream::Attribute> const& rhs
) -> bool {
    if (false == lhs.has_value() && false == rhs.has_value()) {
        return true;
    }
    if (false == lhs.has_value() || false == rhs.has_value()) {
        return false;
    }
    if (lhs.value() != rhs.value()) {
        return false;
    }
    return true;
}
}  // namespace

auto Query::matches_wildcard_queries(std::string_view log_message) const -> bool {
    if (m_wildcard_queries.empty()) {
        return true;
    }
    return std::any_of(
            m_wildcard_queries.begin(),
            m_wildcard_queries.end(),
            [&](auto const& wildcard_query) {
                return wildcard_match_unsafe(
                        log_message,
                        wildcard_query.get_wildcard_query(),
                        wildcard_query.is_case_sensitive()
                );
            }
    );
}

auto Query::matches_attributes(LogEvent::attribute_table_t const& attributes) const -> bool {
    if (m_attribute_queries.empty()) {
        return true;
    }
    auto const attr_end_it{attributes.cend()};
    for (auto const& [query_attr_name, query_attr_val] : m_attribute_queries) {
        auto const it{attributes.find(query_attr_name)};
        if (attr_end_it == it) {
            throw ExceptionFFI(
                    ErrorCode_OutOfBounds,
                    __FILE__,
                    __LINE__,
                    "Attribute name in the query not found: " + query_attr_name
            );
        }
        auto const& attr_val{it->second};
        if (false == compare_attr_val(query_attr_val, attr_val)) {
            return false;
        }
    }
    return true;
}

auto Query::matches_decoded_attributes(
        std::vector<std::optional<ffi::ir_stream::Attribute>> const& decoded_attributes,
        std::unordered_map<std::string, size_t> const& attribute_idx_map
) -> bool {
    if (m_attribute_queries.empty()) {
        return true;
    }
    auto const attr_idx_map_end{attribute_idx_map.cend()};
    for (auto const& [query_attr_name, query_attr_val] : m_attribute_queries) {
        auto const it{attribute_idx_map.find(query_attr_name)};
        if (attr_idx_map_end == it) {
            throw ExceptionFFI(
                    ErrorCode_OutOfBounds,
                    __FILE__,
                    __LINE__,
                    "Attribute name in the query not found: " + query_attr_name
            );
        }
        auto const& attr_val{decoded_attributes.at(it->second)};
        if (false == compare_attr_val(query_attr_val, attr_val)) {
            return false;
        }
    }
    return true;
}
}  // namespace clp_ffi_py::ir::native
