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
 * 时长，用于时间计算
 * @ingroup DataType
 */
class HKU_UTILS_API TimeDelta {
public:
    /**
     * 构造函数
     * @note
     * <pre>
     * 1、总时长范围：TimeDetla(-99999999) - TimeDelta(99999999, 23, 59, 59, 999, 999)。
     * 2、和 boost::posix_time::time_duration 有所区别，time_duration 只要有一个参数为负数，
     *    则认为是负时长，而 TimeDelta 则是各参数代表的ticks数之和。
     * 3、当为负时长时，只有 days() 为负，其余各部分（hours() ...）均为正数。和 python 的
     *    datetime.timedelta 行为一致。
     * </pre>
     * @param days 天数 [-99999999, 99999999]
     * @param hours 小时数 [-100000, 100000]
     * @param minutes 分钟数 [-100000, 100000]
     * @param seconds 秒数 [-8639900, 8639900])
     * @param milliseconds 毫秒数 [-86399000000, 86399000000])
     * @param microseconds 微秒数 [-86399000000, 86399000000])
     */
    explicit TimeDelta(int64_t days = 0, int64_t hours = 0, int64_t minutes = 0,
                       int64_t seconds = 0, int64_t milliseconds = 0, int64_t microseconds = 0);

    /** 通过 boost::posix_time::time_duration 构造 */
    explicit TimeDelta(bt::time_duration td);

    /** 从字符串构造，格式：-1 days, hh:mm:ss.000000) */
    explicit TimeDelta(const std::string& delta);

    /** 赋值构造函数 */
    TimeDelta(const TimeDelta &) = default;

    /** 赋值拷贝函数 */
    TimeDelta &operator=(const TimeDelta &other) {
        if (this != &other) {
            m_duration = other.m_duration;
        }
        return *this;
    }

    /** 获取规范化后的天数 */
    int64_t days() const;

    /** 获取规范化后的小时数 [0, 23] */
    int64_t hours() const;

    /** 获取规范化后的分钟数 [0, 59] */
    int64_t minutes() const;

    /** 获取规范化后的秒数 [0, 59] */
    int64_t seconds() const;

    /** 获取规范化后的毫秒数 [0, 999] */
    int64_t milliseconds() const;

    /** 获取规范化后的微秒数 [0, 999] */
    int64_t microseconds() const;

    /** 获取 tick 数，即转换为微秒后的总微秒数 */
    int64_t ticks() const {
        return m_duration.ticks();
    }

    /** 返回带小数的总天数 */
    double total_days() const {
        return double(ticks()) / 86400000000.0;
    }

    /** 返回带小数的总小时数 */
    double total_hours() const {
        return double(ticks()) / 3600000000.0;
    }

    /** 返回带小数的总分钟数 */
    double total_minutes() const {
        return double(ticks()) / 60000000.0;
    }

    /** 返回带小数的总秒数 */
    double total_seconds() const {
        return double(ticks()) / 1000000.0;
    }

    /** 返回带小数的总毫秒数 */
    double total_milliseconds() const {
        return double(ticks()) / 1000.0;
    }

    /** 是否为负时长 */
    bool isNegative() const {
        return m_duration.is_negative();
    }

    /** 求绝对值 */
    TimeDelta abs() const {
        return TimeDelta::fromTicks(std::abs(ticks()));
    }

    /** 转换为 boost::posix_time::time_duration */
    bt::time_duration time_duration() const {
        return m_duration;
    }

    /** 转换为字符串，格式：-1 days hh:mm:ss.000000) */
    std::string str() const {
#if FMT_VERSION >= 90000
        return fmt::format("{} days, {:0>2d}:{:0>2d}:{:<2.6f}", days(), hours(), minutes(),
                           seconds() + double(milliseconds() * 1000 + microseconds()) * 0.000001);
#else
        return fmt::format("{} days, {:>02d}:{:>02d}:{:<2.6f}", days(), hours(), minutes(),
                           seconds() + double(milliseconds() * 1000 + microseconds()) * 0.000001);
#endif
    }

    /** 转换为字符串，格式为：TimeDelta(days,hours,mins,secs,millisecs,microsecs) */
    std::string repr() const {
        return fmt::format("TimeDelta({}, {}, {}, {}, {}, {})", days(), hours(), minutes(),
                           seconds(), milliseconds(), microseconds());
    }

    /////////////////////////////////////////////////////////////////
    //
    // 运算符重载，TimeDelta 相关运算
    //
    /////////////////////////////////////////////////////////////////

    /** 两个时长相加 */
    TimeDelta operator+(TimeDelta td) const {
        return TimeDelta(td.m_duration + m_duration);
    }

    /** 两个时长相减 */
    TimeDelta operator-(TimeDelta td) const {
        return TimeDelta(m_duration - td.m_duration);
    }

    /** + 号,返回相同值 */
    TimeDelta operator+() const {
        return *this;
    }

    /** - 号, 求负值，相当于 TimeDelta(0) 减自身 */
    TimeDelta operator-() const {
        return TimeDelta::fromTicks(-ticks());
    }

    /** 时长乘以系数，结果四舍五入 */
    TimeDelta operator*(double p) const;

    /**
     * 时长除以系数，结果四舍五入。如果除零，将抛出 hku::exception。
     * 如果不希望四舍五入，请使用 floorDiv
     */
    TimeDelta operator/(double p) const;

    /** 两个时长相除，求两者比例。如果除以零时长，将抛出 hku::exception。 */
    double operator/(TimeDelta td) const;

    /** 地板除，小数点后直接截断 */
    TimeDelta floorDiv(double p) const;

    /** 两个时长相除求余。如果除以零时长，将抛出 hku::exception。 */
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

    /////////////////////////////////////////////////////////////////
    //
    // 静态成员函数
    //
    /////////////////////////////////////////////////////////////////

    /** 获取能够表达的最小值 TimeDelta(-99999999) */
    static TimeDelta min() {
        return TimeDelta(-99999999);
    }

    /** 获取能够表达的最大值 TimeDelta(99999999, 23, 59, 59, 999, 999) */
    static TimeDelta max() {
        return TimeDelta(99999999, 23, 59, 59, 999, 999);
    }

    /** 支持的最大 tick 数 */
    static int64_t maxTicks() {
        return m_max_micro_seconds;
    }

    /** 支持的最小 tick 数 */
    static int64_t minTicks() {
        return m_min_micro_seconds;
    }

    /** 获取表达精度 1 微秒, TimeDelta(0, 0, 0, 0, 0, 1) */
    static TimeDelta resolution() {
        return TimeDelta(0, 0, 0, 0, 0, 1);
    }

    /** 从 ticks 创建 */
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
 * TimeDelta 快捷创建函数
 * @param days 天数 [-99999999, 99999999]
 * @ingroup DataType
 */
TimeDelta Days(int64_t days);
inline TimeDelta Days(int64_t days) {
    return TimeDelta(days);
}

/**
 * TimeDelta 快捷创建函数
 * @param hours 小时数
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Hours(int64_t hours);

/**
 * TimeDelta 快捷创建函数
 * @param mins 分钟数
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Minutes(int64_t mins);

/**
 * TimeDelta 快捷创建函数
 * @param secs 秒数
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Seconds(int64_t secs);

/**
 * TimeDelta 快捷创建函数
 * @param milliseconds 毫秒数
 * @ingroup DataType
 */
TimeDelta HKU_UTILS_API Milliseconds(int64_t milliseconds);

/**
 * TimeDelta 快捷创建函数
 * @param microsecs 微秒数
 * @ingroup DataType
 */
TimeDelta Microseconds(int64_t microsecs);
inline TimeDelta Microseconds(int64_t microsecs) {
    return TimeDelta::fromTicks(microsecs);
}

} /* namespace hku */

namespace std {

inline string to_string(const hku::TimeDelta &delta) {
    return delta.str();
}

}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TimeDelta> {
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const hku::TimeDelta &d, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};
#endif

#endif /* HIKYUU_DATETIME_TIMEDELTA_H */