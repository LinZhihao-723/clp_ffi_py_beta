#ifndef CLP_FFI_PY_QUERY
#define CLP_FFI_PY_QUERY

#include <clp_ffi_py/decoder/Message.hpp>

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <limits>
#include <vector>

namespace clp_ffi_py::decoder {
class Query {
public:
    static constexpr ffi::epoch_time_ms_t const cDefaultTimestampLowerBound = 0;
    static constexpr ffi::epoch_time_ms_t const cDefaultTimestampUpperBound =
            std::numeric_limits<ffi::epoch_time_ms_t>::max();

    static constexpr ffi::epoch_time_ms_t const cTimestampUpperBoundSafeRange = 60 * 1000;

    Query(bool case_sensitive)
        : m_case_sensitive{case_sensitive},
          m_ts_lower_bound{cDefaultTimestampLowerBound},
          m_ts_upper_bound{cDefaultTimestampUpperBound} {};

    Query(bool case_sensitive,
          ffi::epoch_time_ms_t ts_lower_bound,
          ffi::epoch_time_ms_t ts_upper_bound)
        : m_case_sensitive{case_sensitive},
          m_ts_lower_bound{ts_lower_bound},
          m_ts_upper_bound{ts_upper_bound} {};

    void add_query(std::string_view wildcard) noexcept { m_query_list.emplace_back(wildcard); }

    [[nodiscard]] auto get_query_list_const_ref() const -> std::vector<std::string> const& {
        return m_query_list;
    }

    [[nodiscard]] auto is_case_sensitive() const -> bool { return m_case_sensitive; }

    [[nodiscard]] auto get_ts_lower_bound() const -> ffi::epoch_time_ms_t {
        return m_ts_lower_bound;
    }

    [[nodiscard]] auto get_ts_upper_bound() const -> ffi::epoch_time_ms_t {
        return m_ts_upper_bound;
    }

    void set_ts_lower_bound(ffi::epoch_time_ms_t ts) { m_ts_lower_bound = ts; }

    void set_ts_upper_bound(ffi::epoch_time_ms_t ts) { m_ts_upper_bound = ts; }

    [[nodiscard]] auto ts_lower_bound_check(ffi::epoch_time_ms_t ts) const -> bool {
        return ts >= m_ts_lower_bound;
    }

    [[nodiscard]] auto ts_upper_bound_check(ffi::epoch_time_ms_t ts) const -> bool {
        return ts <= m_ts_upper_bound;
    }

    [[nodiscard]] auto ts_upper_bound_exit_check(ffi::epoch_time_ms_t ts) const -> bool {
        return m_ts_upper_bound - ts >= cTimestampUpperBoundSafeRange;
    }

    [[nodiscard]] auto ts_in_range(ffi::epoch_time_ms_t ts) const -> bool {
        return ts_lower_bound_check(ts) && ts_upper_bound_check(ts);
    }

    [[nodiscard]] auto matches(Message const& message) const -> bool;

    [[nodiscard]] auto matches(std::string_view message) const -> bool;

private:
    std::vector<std::string> m_query_list;
    bool const m_case_sensitive;

    ffi::epoch_time_ms_t m_ts_lower_bound;
    ffi::epoch_time_ms_t m_ts_upper_bound;
};
} // namespace clp_ffi_py::decoder
#endif
