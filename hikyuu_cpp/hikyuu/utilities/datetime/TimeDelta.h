/*
 * TimeDelta.h
 *
 *  Copyright(C) 2019, hikyuu.org
 *
 *  Created on: 2019-12-12
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DATETIME_TIMEDELTA_H
#define HIKYUU_DATETIME_TIMEDELTA_H

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

/**
 * Duration, used for the time calculation
 * @ingroup DataType
 */
class HKU_UTILS_API TimeDelta {
public:
    /**
     * Constructor
     * @note
     * <pre>
     * 1. Total duration range: TimeDetla(-99999999) - TimeDelta(99999999, 23, 59, 59, 999, 999).
     * 2. It differs from boost::posix_time::time_duration: as long as one parameter of
     * time_duration is negative it is regarded as a negative duration, while TimeDelta is the sum
     * of the ticks represented by every parameter.
     * 3. When it is a negative duration only days() is negative, all the other parts (hours() ...)
     *    are positive. This is consistent with the behavior of python datetime.timedelta.
     * </pre>
     * @param days days [-99999999, 99999999]
     * @param hours hours [-100000, 100000]
     * @param minutes minutes [-100000, 100000]
     * @param seconds seconds [-8639900, 8639900])
     * @param milliseconds milliseconds [-86399000000, 86399000000])
     * @param microseconds microseconds [-86399000000, 86399000000])
     */
    explicit TimeDelta(int64_t days = 0, int64_t hours = 0, int64_t minutes = 0,
                       int64_t seconds = 0, int64_t milliseconds = 0, int64_t microseconds = 0);

    /** Construct from boost::posix_time::time_duration */
    explicit TimeDelta(bt::time_duration td);

    /**
     * Construct from std::chrono::duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param dur the chrono duration object
     */
    template <typename Rep, typename Period>
    explicit TimeDelta(std::chrono::duration<Rep, Period> dur) {
        // Convert to microseconds
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(dur);
        int64_t total = microseconds.count();
        if (total < m_min_micro_seconds || total > m_max_micro_seconds) {
            throw std::out_of_range("TimeDelta value out of range");
        }
        m_duration = bt::time_duration(0, 0, 0, total);
    }

    /** Construct from a string, the format: -1 days, hh:mm:ss.000000) */
    explicit TimeDelta(const std::string &delta);

    /** Assignment constructor */
    TimeDelta(const TimeDelta &) = default;

    /** Copy assignment function */
    TimeDelta &operator=(const TimeDelta &other) {
        if (this != &other) {
            m_duration = other.m_duration;
        }
        return *this;
    }

    /** Get the normalized number of the days */
    int64_t days() const;

    /** Get the normalized hours [0, 23] */
    int64_t hours() const;

    /** Get the normalized minutes [0, 59] */
    int64_t minutes() const;

    /** Get the normalized seconds [0, 59] */
    int64_t seconds() const;

    /** Get the normalized milliseconds [0, 999] */
    int64_t milliseconds() const;

    /** Get the normalized microseconds [0, 999] */
    int64_t microseconds() const;

    /** Get the number of the ticks, i.e. the total microseconds after the conversion */
    int64_t ticks() const {
        return m_duration.ticks();
    }

    /** Return the total days with a fraction */
    double total_days() const {
        return double(ticks()) / 86400000000.0;
    }

    /** Return the total hours with a fraction */
    double total_hours() const {
        return double(ticks()) / 3600000000.0;
    }

    /** Return the total minutes with a fraction */
    double total_minutes() const {
        return double(ticks()) / 60000000.0;
    }

    /** Return the total seconds with a fraction */
    double total_seconds() const {
        return double(ticks()) / 1000000.0;
    }

    /** Return the total milliseconds with a fraction */
    double total_milliseconds() const {
        return double(ticks()) / 1000.0;
    }

    /** Whether it is a negative duration */
    bool isNegative() const {
        return m_duration.is_negative();
    }

    /** Calculate the absolute value */
    TimeDelta abs() const {
        return TimeDelta::fromTicks(std::abs(ticks()));
    }

    /** Convert to boost::posix_time::time_duration */
    bt::time_duration time_duration() const {
        return m_duration;
    }

    /**
     * Convert to std::chrono::duration
     * @tparam Duration the target chrono duration type, microseconds by default
     * @return the converted chrono duration
     */
    template <typename Duration = std::chrono::microseconds>
    Duration duration() const {
        return std::chrono::duration_cast<Duration>(std::chrono::microseconds(m_duration.ticks()));
    }

    /** Convert to a string, the format: -1 days hh:mm:ss.000000) */
    std::string str() const {
#if FMT_VERSION >= 90000
        return fmt::format("{} days, {:0>2d}:{:0>2d}:{:<2.6f}", days(), hours(), minutes(),
                           seconds() + double(milliseconds() * 1000 + microseconds()) * 0.000001);
#else
        return fmt::format("{} days, {:>02d}:{:>02d}:{:<2.6f}", days(), hours(), minutes(),
                           seconds() + double(milliseconds() * 1000 + microseconds()) * 0.000001);
#endif
    }

    /** Convert to a string, the format: TimeDelta(days,hours,mins,secs,millisecs,microsecs) */
    std::string repr() const {
        return fmt::format("TimeDelta({}, {}, {}, {}, {}, {})", days(), hours(), minutes(),
                           seconds(), milliseconds(), microseconds());
    }

    /////////////////////////////////////////////////////////////////
    //
    // Operator overloads, the TimeDelta related operations
    //
    /////////////////////////////////////////////////////////////////

    /** Add two durations */
    TimeDelta operator+(TimeDelta td) const {
        return TimeDelta(td.m_duration + m_duration);
    }

    /** Subtract two durations */
    TimeDelta operator-(TimeDelta td) const {
        return TimeDelta(m_duration - td.m_duration);
    }

    /** The + operator, it returns the same value */
    TimeDelta operator+() const {
        return *this;
    }

    /** The - operator, it gives the negative value, equivalent to TimeDelta(0) minus itself */
    TimeDelta operator-() const {
        return TimeDelta::fromTicks(-ticks());
    }

    /** Multiply the duration by a coefficient, the result is rounded */
    TimeDelta operator*(double p) const;

    /**
     * Divide the duration by a coefficient, the result is rounded. hku::exception is thrown if it
     * is divided by zero.
     * Use floorDiv if the rounding is not desired
     */
    TimeDelta operator/(double p) const;

    /** Divide two durations to get their ratio. hku::exception is thrown when it is divided by a
     *  zero duration. */
    double operator/(TimeDelta td) const;

    /** Floor division, the decimal part is truncated directly */
    TimeDelta floorDiv(double p) const;

    /** The remainder of the division of two durations. hku::exception is thrown when it is divided
     *  by a zero duration. */
    TimeDelta operator%(TimeDelta td) const;

    bool operator==(TimeDelta td) const {
        return m_duration == td.m_duration;
    }

    bool operator!=(TimeDelta td) const {
        return m_duration != td.m_duration;
    }

    bool operator>(TimeDelta td) const {
        return m_duration > td.m_duration;
    }

    bool operator<(TimeDelta td) const {
        return m_duration < td.m_duration;
    }

    bool operator>=(TimeDelta td) const {
        return m_duration >= td.m_duration;
    }

    bool operator<=(TimeDelta td) const {
        return m_duration <= td.m_duration;
    }

    /**
     * Add a chrono duration to the duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return TimeDelta
     */
    template <typename Rep, typename Period>
    TimeDelta operator+(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        auto total_ticks = ticks() + duration_us.count();
        return TimeDelta::fromTicks(total_ticks);
    }

    /**
     * Subtract a chrono duration from the duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return TimeDelta
     */
    template <typename Rep, typename Period>
    TimeDelta operator-(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        auto total_ticks = ticks() - duration_us.count();
        return TimeDelta::fromTicks(total_ticks);
    }

    /**
     * Compare whether it is equal to a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return bool
     */
    template <typename Rep, typename Period>
    bool operator==(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        return ticks() == duration_us.count();
    }

    /**
     * Compare whether it is not equal to a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return bool
     */
    template <typename Rep, typename Period>
    bool operator!=(std::chrono::duration<Rep, Period> duration) const {
        return !(*this == duration);
    }

    /**
     * Compare whether it is greater than a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return bool
     */
    template <typename Rep, typename Period>
    bool operator>(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        return ticks() > duration_us.count();
    }

    /**
     * Compare whether it is less than a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return bool
     */
    template <typename Rep, typename Period>
    bool operator<(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        return ticks() < duration_us.count();
    }

    /**
     * Compare whether it is greater than or equal to a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return bool
     */
    template <typename Rep, typename Period>
    bool operator>=(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        return ticks() >= duration_us.count();
    }

    /**
     * Compare whether it is less than or equal to a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return bool
     */
    template <typename Rep, typename Period>
    bool operator<=(std::chrono::duration<Rep, Period> duration) const {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        return ticks() <= duration_us.count();
    }

    /////////////////////////////////////////////////////////////////
    //
    // Static member functions
    //
    /////////////////////////////////////////////////////////////////

    /** Get the minimum expressible value TimeDelta(-99999999) */
    static TimeDelta min() {
        return TimeDelta(-99999999);
    }

    /** Get the maximum expressible value TimeDelta(99999999, 23, 59, 59, 999, 999) */
    static TimeDelta max() {
        return TimeDelta(99999999, 23, 59, 59, 999, 999);
    }

    /** The maximum number of the ticks supported */
    static int64_t maxTicks() {
        return m_max_micro_seconds;
    }

    /** The minimum number of the ticks supported */
    static int64_t minTicks() {
        return m_min_micro_seconds;
    }

    /** Get the expression precision 1 microsecond, TimeDelta(0, 0, 0, 0, 0, 1) */
    static TimeDelta resolution() {
        return TimeDelta(0, 0, 0, 0, 0, 1);
    }

    /** Create from the ticks */
    static TimeDelta fromTicks(int64_t ticks);

private:
    bt::time_duration m_duration;

    static constexpr const int64_t m_max_micro_seconds = 100000000LL * 24 * 60 * 60 * 1000000 - 1;
    static constexpr const int64_t m_min_micro_seconds = -99999999LL * 24 * 60 * 60 * 1000000;
    static constexpr const int64_t m_one_day_ticks = 24 * 60 * 60 * 1000000LL;
};

std::ostream &operator<<(std::ostream &out, TimeDelta td);
inline std::ostream &operator<<(std::ostream &out, TimeDelta td) {
    out << td.str();
    return out;
}

/**
 * The reverse operator of chrono duration + TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return TimeDelta
 */
template <typename Rep, typename Period>
inline TimeDelta operator+(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td + duration;
}

/**
 * The reverse operator of chrono duration - TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return TimeDelta
 */
template <typename Rep, typename Period>
inline TimeDelta operator-(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    auto total_ticks = duration_us.count() - td.ticks();
    return TimeDelta::fromTicks(total_ticks);
}

/**
 * The reverse comparison of chrono duration == TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return bool
 */
template <typename Rep, typename Period>
inline bool operator==(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td == duration;
}

/**
 * The reverse comparison of chrono duration != TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return bool
 */
template <typename Rep, typename Period>
inline bool operator!=(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td != duration;
}

/**
 * The reverse comparison of chrono duration < TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return bool
 */
template <typename Rep, typename Period>
inline bool operator<(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td > duration;
}

/**
 * The reverse comparison of chrono duration > TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return bool
 */
template <typename Rep, typename Period>
inline bool operator>(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td < duration;
}

/**
 * The reverse comparison of chrono duration <= TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return bool
 */
template <typename Rep, typename Period>
inline bool operator<=(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td >= duration;
}

/**
 * The reverse comparison of chrono duration >= TimeDelta
 * @tparam Rep the representation type
 * @tparam Period the period type
 * @param duration chrono duration
 * @param td TimeDelta
 * @return bool
 */
template <typename Rep, typename Period>
inline bool operator>=(std::chrono::duration<Rep, Period> duration, TimeDelta td) {
    return td <= duration;
}

/**
 * Quick creation function of TimeDelta
 * @param days days [-99999999, 99999999]
 * @ingroup DataType
 */
TimeDelta Days(int64_t days);
inline TimeDelta Days(int64_t days) {
    return TimeDelta(days);
}

/**
 * Quick creation function of TimeDelta
 * @param hours hours
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Hours(int64_t hours);

/**
 * Quick creation function of TimeDelta
 * @param mins minutes
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Minutes(int64_t mins);

/**
 * Quick creation function of TimeDelta
 * @param secs seconds
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Seconds(int64_t secs);

/**
 * Quick creation function of TimeDelta
 * @param milliseconds milliseconds
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Milliseconds(int64_t milliseconds);

/**
 * Quick creation function of TimeDelta
 * @param microsecs microseconds
 * @ingroup DataType
 */
TimeDelta Microseconds(int64_t microsecs);
inline TimeDelta Microseconds(int64_t microsecs) {
    return TimeDelta::fromTicks(microsecs);
}

/**
 * Get the current UTC time offset
 * @return TimeDelta
 */
TimeDelta HKU_UTILS_API UTCOffset();

} /* namespace hku */

namespace std {

template <>
class hash<hku::TimeDelta> {
public:
    size_t operator()(hku::TimeDelta const &d) const noexcept {
        return std::hash<int64_t>()(d.ticks());
    }
};

inline string to_string(const hku::TimeDelta &delta) {
    return delta.str();
}

}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TimeDelta> {
    FMT_CONSTEXPR auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const hku::TimeDelta &d, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};
#endif

#endif /* HIKYUU_DATETIME_TIMEDELTA_H */