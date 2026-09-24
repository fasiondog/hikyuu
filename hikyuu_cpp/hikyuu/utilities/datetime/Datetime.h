/*
 * Datetime.h
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#pragma once
#ifndef DATETIME_H_
#define DATETIME_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <chrono>
#include <string>
#include <vector>
#include <fmt/ostream.h>

#include "hikyuu/utilities/config.h"
#if !HKU_SUPPORT_DATETIME
#error "Don't support datetime, you can config with --datetime=y"
#endif
#include "TimeDelta.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4251)
#endif

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

/**
 * Date type
 * @details An exception std::out_of_range is thrown when the construction fails
 * @ingroup DataType
 */
class HKU_UTILS_API Datetime {
public:
    /** Return the minimum expressible date: 1400-Jan-01 00:00:00 */
    static Datetime min();

    /** Return the maximum expressible date: 9999-Dec-31 00:00:00 */
    static Datetime max();

    /** Return the current moment of the local machine */
    static Datetime now();

    /** Return today's date of the local machine */
    static Datetime today();

    /**
     * Compatible with oracle
     * datetime format: except the most significant byte, every byte represents the century, the
     * year in the century, the month, the day, the hour, the minute and the second in turn
     */
    static Datetime fromHex(uint64_t time);

    /**
     * @brief Create a Datetime object from a timestamp (microseconds)
     * @param timestamp the timestamp in microseconds (starting from 00:00:00 on January 1, 1970)
     * @return Datetime
     */
    static Datetime fromTimestamp(int64_t timestamp);

    /**
     * @brief Create a Datetime object from a timestamp (microseconds) and add the local UTC time
     * offset
     * @param timestamp the timestamp in microseconds (starting from 00:00:00 on January 1, 1970)
     * @return Datetime
     */
    static Datetime fromTimestampUTC(int64_t timestamp);

    /**
     * @brief Create a Datetime object from std::chrono::local_time
     * @tparam Duration the chrono duration type
     * @param local_time chrono local_time
     * @return Datetime
     */
    template <typename Duration>
    static Datetime fromLocalTime(std::chrono::local_time<Duration> local_time);

    /**
     * @brief Create a Datetime object from std::chrono::time_point and add the local UTC time
     * offset
     * @tparam Clock the clock type
     * @tparam Duration the chrono duration type
     * @param time_point chrono time_point (usually representing a UTC time)
     * @return Datetime
     */
    template <typename Clock, typename Duration>
    static Datetime fromTimePointUTC(std::chrono::time_point<Clock, Duration> time_point);

public:
    /** Default constructor, Null<Datetime> */
    Datetime();

    Datetime(const Datetime &);

    /**
     * Constructor
     * @param year year
     * @param month month
     * @param day day
     * @param hh hour
     * @param mm minute
     * @param sec second
     * @param millisec millisecond
     * @param microsec microsecond
     */
    Datetime(long year, long month, long day, long hh = 0, long mm = 0, long sec = 0,
             long millisec = 0, long microsec = 0);

    /** Construct the date type from boost::gregorian::date */
    explicit Datetime(const bd::date &);

    /** Construct from boost::posix_time::ptime */
    explicit Datetime(const bt::ptime &);

    /**
     * Construct the date type with a number
     * @details The following number formats are supported
     * <pre>
     *     1. YYYYMMDDhhmm, e.g. 200101010000
     *     2. YYYYMMDD, e.g. 20010101
     *     3. YYYYMMDDhhmmss, e.g. 20010101000000
     * </pre>
     */
    explicit Datetime(unsigned long long);

    /**
     * Construct the date type with a string
     * @details The following string formats are supported for the construction:
     * <pre>
     *     1. "2001-01-01" or "2001/1/1"
     *     2、"20010101"
     *     3、"2001-01-01 18:00:00.12345"
     *     4、"20010101T181159"
     *     5、"20240822 11:30:06.230"
     * </pre>
     */
    explicit Datetime(const std::string &);

    Datetime &operator=(const Datetime &);

    /** Year; an exception is thrown if it is Null */
    long year() const;

    /** Month [1, 12]; an exception is thrown if it is Null */
    long month() const;

    /** Day [1, 31]; an exception is thrown if it is Null */
    long day() const;

    /** Hour [0, 23]; an exception is thrown if it is Null */
    long hour() const;

    /** Minute [0, 59]; an exception is thrown if it is Null */
    long minute() const;

    /** Second [0, 59]; an exception is thrown if it is Null */
    long second() const;

    /** Millisecond [0, 999]; an exception is thrown if it is Null */
    long millisecond() const;

    /** Microsecond [0, 999]; an exception is thrown if it is Null */
    long microsecond() const;

    /** Whether it is Null<Datetime> */
    bool isNull() const;

    /** Date operation, add the given duration */
    Datetime operator+(TimeDelta d) const;

    /** Date operation, subtract the given duration */
    Datetime operator-(TimeDelta d) const;

    /**
     * Date operation, add a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return Datetime
     */
    template <typename Rep, typename Period>
    Datetime operator+(std::chrono::duration<Rep, Period> duration) const;

    /**
     * Date operation, subtract a chrono duration
     * @tparam Rep the representation type
     * @tparam Period the period type
     * @param duration chrono duration
     * @return Datetime
     */
    template <typename Rep, typename Period>
    Datetime operator-(std::chrono::duration<Rep, Period> duration) const;

    /**
     * Return a number in the YYYYMMDDhhmm format, convenient for the comparison operations
     * The number corresponding to Null<Datetime>() is Null<unsigned long long>
     * @note The precision is up to the minute
     */
    uint64_t number() const noexcept;

    /** Return a number in the YYYYMMDD format */
    uint64_t ym() const noexcept;

    /** Return a number in the YYYYMMDD format */
    uint64_t ymd() const noexcept;

    /** Return a number in the YYYYMMDDHH format */
    uint64_t ymdh() const noexcept;

    /** Return a number in the YYYYMMDDhhmm format */
    uint64_t ymdhm() const noexcept;

    /** Return a number in the YYYYMMDDhhmmss format */
    uint64_t ymdhms() const noexcept;

    /**
     * Convert to a number in the oracle datetime way; the last 7 bytes represent the century, the
     * year in the century, the month, the day, the hour, the minute and the second respectively
     * @note The precision is up to the second
     */
    uint64_t hex() const noexcept;

    /** The number of the microseconds elapsed since the minimum date */
    uint64_t ticks() const noexcept;

    /** Timestamp, the number of the microseconds since 1970-01-01 00:00:00 */
    uint64_t timestamp() const noexcept;

    /** Timestamp, the number of the microseconds since 1970-01-01 00:00:00, with the local UTC
     *  offset deducted */
    uint64_t timestampUTC() const noexcept;

    /**
     * Convert to a string for the reading and printing, the format is:
     * <pre>
     * When the milliseconds and microseconds are zero: 2019-01-02 01:01:00
     * When they are not zero: 2019-01-02 01:01:00:000001
     * </pre>
     */
    std::string str() const;

    /**
     * Convert to a string,
     * the format is: Datetime(year, month, day, hour, minute, second, millisecond, microsecond)
     */
    std::string repr() const;

    /** Return boost::posix_time::ptime */
    bt::ptime ptime() const;

    /** Return boost::gregorian::date */
    bd::date date() const;

    /** Return std::time_t */
    std::time_t to_time_t() const;

    /**
     * Convert to std::chrono::local_time
     * @tparam Duration the target chrono duration type, microseconds by default
     * @return chrono local_time
     */
    template <typename Duration = std::chrono::microseconds>
    std::chrono::local_time<Duration> to_local_time() const;

    /** Return the day of the week, Sunday is 0 and Monday is 1 */
    int dayOfWeek() const;

    /** Return the day of the year, January 1 is the first day of the year */
    int dayOfYear() const;

    /** The start date of the day, i.e. 0 o'clock */
    Datetime startOfDay() const;

    /** The end date of the day, i.e. 23:59:59 */
    Datetime endOfDay() const;

    /**
     * Return the date of the given day of this week; Sunday is day 0 and Saturday is day 6
     * @param day the day of this week; Null is returned if it is not between 0 and 6
     */
    Datetime dateOfWeek(int day) const;

    /** Return the start date of the week (Monday) */
    Datetime startOfWeek() const;

    /** Return the end date of the week (Sunday) */
    Datetime endOfWeek() const;

    /** Return the start date of the month */
    Datetime startOfMonth() const;

    /** Return the end date of the month, such as December 31 */
    Datetime endOfMonth() const;

    /** Return the start date of the quarter */
    Datetime startOfQuarter() const;

    /** Return the end date of the quarter */
    Datetime endOfQuarter() const;

    /** Return the start date of the half year */
    Datetime startOfHalfyear() const;

    /** Return the end date of the half year */
    Datetime endOfHalfyear() const;

    /** Return the start date of the year */
    Datetime startOfYear() const;

    /** Return the end date of the year */
    Datetime endOfYear() const;

    /** The next natural day */
    Datetime nextDay() const;

    /** The start date of the next week (Monday) */
    Datetime nextWeek() const;

    /** The start date of the next month */
    Datetime nextMonth() const;

    /** The start date of the next quarter */
    Datetime nextQuarter() const;

    /** The start date of the next half year */
    Datetime nextHalfyear() const;

    /** The start date of the next year */
    Datetime nextYear() const;

    /** The previous natural day */
    Datetime preDay() const;

    /** The Monday date of the previous week */
    Datetime preWeek() const;

    /** The start date of the previous month */
    Datetime preMonth() const;

    /** The start date of the previous quarter */
    Datetime preQuarter() const;

    /** The start date of the previous half year */
    Datetime preHalfyear() const;

    /** The start date of the previous year */
    Datetime preYear() const;

private:
    bt::ptime m_data;
};

HKU_UTILS_API std::ostream &operator<<(std::ostream &, const Datetime &);

/**
 * Date list
 * @ingroup DataType
 */
typedef std::vector<Datetime> DatetimeList;

/**
 * Get the calendar date list within the given range [start,end), it contains the days only, without
 * the hour, minute and second.
 * @note A large amount of memory may be consumed if the given end date is too large
 * @param start start date
 * @param end end date; Datetime::max is used when it is empty
 * @return the calendar dates within the range [start, end)
 */
DatetimeList HKU_UTILS_API getDateRange(const Datetime &start, const Datetime &end);

///////////////////////////////////////////////////////////////////////////////
//
// The relational comparison functions are not defined in the class directly in order to support
// Null<>() == d, i.e. Null can be placed on the left
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const Datetime &, const Datetime &);
bool operator!=(const Datetime &, const Datetime &);
bool operator>(const Datetime &, const Datetime &);
bool operator<(const Datetime &, const Datetime &);
bool operator>=(const Datetime &, const Datetime &);
bool operator<=(const Datetime &, const Datetime &);

inline bool operator==(const Datetime &d1, const Datetime &d2) {
    return d1.ptime() == d2.ptime();
}

inline bool operator!=(const Datetime &d1, const Datetime &d2) {
    return d1.ptime() != d2.ptime();
}

inline bool operator>(const Datetime &d1, const Datetime &d2) {
    return d1.ptime() > d2.ptime();
}

inline bool operator<(const Datetime &d1, const Datetime &d2) {
    return d1.ptime() < d2.ptime();
}

inline bool operator>=(const Datetime &d1, const Datetime &d2) {
    return d1.ptime() >= d2.ptime();
}

inline bool operator<=(const Datetime &d1, const Datetime &d2) {
    return d1.ptime() <= d2.ptime();
}

///////////////////////////////////////////////////////////////////////////////
//
// Supplement of the addition and subtraction operations
//
///////////////////////////////////////////////////////////////////////////////
inline Datetime operator+(const TimeDelta &delta, const Datetime &date) {
    return date + delta;
}

template <typename Rep, typename Period>
inline Datetime operator+(std::chrono::duration<Rep, Period> duration, const Datetime &date) {
    return date + TimeDelta(duration);
}

inline TimeDelta operator-(const Datetime &d1, const Datetime &d2) {
    return TimeDelta(d1.ptime() - d2.ptime());
}

///////////////////////////////////////////////////////////////////////////////
//
// Definition of the inline member functions
//
///////////////////////////////////////////////////////////////////////////////

inline Datetime::Datetime() {
    bd::date d(bd::pos_infin);
    m_data = bt::ptime(d, bt::time_duration(0, 0, 0));
}

inline Datetime::Datetime(const Datetime &d) : m_data(d.m_data) {}

inline Datetime::Datetime(const bd::date &d) : m_data(bt::ptime(d, bt::time_duration(0, 0, 0))) {}

inline Datetime::Datetime(const bt::ptime &d) : m_data(d) {}

template <typename Duration>
inline Datetime Datetime::fromLocalTime(std::chrono::local_time<Duration> local_time) {
    if (local_time == std::chrono::local_time<Duration>::max()) {
        return Datetime();
    }
    auto duration_us =
      std::chrono::duration_cast<std::chrono::microseconds>(local_time.time_since_epoch());
    return fromTimestamp(static_cast<int64_t>(duration_us.count()));
}

template <typename Clock, typename Duration>
inline Datetime Datetime::fromTimePointUTC(std::chrono::time_point<Clock, Duration> time_point) {
    if (time_point == std::chrono::time_point<Clock, Duration>::max()) {
        return Datetime();
    }
    auto duration_us =
      std::chrono::duration_cast<std::chrono::microseconds>(time_point.time_since_epoch());
    return fromTimestampUTC(static_cast<int64_t>(duration_us.count()));
}

inline bt::ptime Datetime::ptime() const {
    return m_data;
}

inline bd::date Datetime::date() const {
    return m_data.date();
}

inline std::time_t Datetime::to_time_t() const {
    std::tm tt = bt::to_tm(m_data);
    return std::mktime(&tt);
}

template <typename Duration>
inline std::chrono::local_time<Duration> Datetime::to_local_time() const {
    if (isNull()) {
        return std::chrono::local_time<Duration>::max();
    }
    auto timestamp_us = static_cast<int64_t>(timestamp());
    auto target_duration =
      std::chrono::duration_cast<Duration>(std::chrono::microseconds(timestamp_us));
    return std::chrono::local_time<Duration>(target_duration);
}

inline int Datetime::dayOfWeek() const {
    return date().day_of_week();
}

inline int Datetime::dayOfYear() const {
    return date().day_of_year();
}

inline Datetime Datetime::startOfDay() const {
    return Datetime(date());
}

inline Datetime Datetime::operator+(TimeDelta d) const {
    return Datetime(m_data + d.time_duration());
}

inline Datetime Datetime::operator-(TimeDelta d) const {
    return Datetime(m_data - d.time_duration());
}

template <typename Rep, typename Period>
inline Datetime Datetime::operator+(std::chrono::duration<Rep, Period> duration) const {
    return *this + TimeDelta(duration);
}

template <typename Rep, typename Period>
inline Datetime Datetime::operator-(std::chrono::duration<Rep, Period> duration) const {
    return *this - TimeDelta(duration);
}

} /* namespace hku */

namespace std {
template <>
class hash<hku::Datetime> {
public:
    size_t operator()(hku::Datetime const &d) const noexcept {
        return std::hash<uint64_t>()(d.ticks());
    }
};

inline string to_string(const hku::Datetime &date) {
    return date.str();
}

}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Datetime> {
    FMT_CONSTEXPR auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const hku::Datetime &d, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};
#endif

#endif /* DATETIME_H_ */
