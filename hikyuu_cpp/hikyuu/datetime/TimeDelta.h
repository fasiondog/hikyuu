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

#include <cstdint>
#include <fmt/format.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

/**
 * 持续时长，用于时间计算
 * @ingroup DataType
 */
class HKU_API TimeDelta {
public:
    /**
     * 构造函数
     * @note 总时长不能超过 TimeDelta(99999999, 23, 59, 59, 999, 999)
     * @param days 天数 [-99999999, 99999999]
     * @param hours 小时数 [-10000, 10000]
     * @param minutes 分钟数 [-50000, 50000]
     * @param seconds 秒数 [-86399, 86399]
     * @param milliseconds 毫秒数 [-999, 999]
     * @param microseconds 微秒数 [-999, 999]
     */
    TimeDelta(int64_t days = 0, int64_t hours = 0, int64_t minutes = 0, int64_t seconds = 0,
              int64_t milliseconds = 0, int64_t microseconds = 0);

    /** 通过 boost::posix_time::time_duration 构造 */
    explicit TimeDelta(bt::time_duration td);

    /** 赋值构造函数 */
    TimeDelta(const TimeDelta&) = default;

    /** 赋值拷贝函数 */
    TimeDelta& operator=(const TimeDelta& other) {
        if (this != &other) {
            m_duration = other.m_duration;
        }
        return *this;
    }

    /** 获取规范化后的天数 */
    int64_t days() const {
        int64_t result = m_duration.hours() / 24;
        if (isNegative()) {
            result = -1 - result;
        }
        return result;
    }

    /** 获取规范化后的小时数 [0, 23] */
    int64_t hours() const {
        return std::abs(m_duration.hours()) % 24;
    }

    /** 获取规范化后的分钟数 [0, 59] */
    int64_t minutes() const {
        return std::abs(m_duration.minutes());
    }

    /** 获取规范化后的秒数 [0, 59] */
    int64_t seconds() const {
        return std::abs(m_duration.seconds());
    }

    /** 获取规范化后的毫秒数 [0, 999] */
    int64_t milliseconds() const {
        return (std::abs(ticks()) % 1000000 - microseconds()) / 1000;
    }

    /** 获取规范化后的微秒数 [0, 999] */
    int64_t microseconds() const {
        return std::abs(ticks() % 1000);
    }

    /** 获取 tick 数，即转换为微秒后的总微秒数 */
    int64_t ticks() const {
        return m_duration.ticks();
    }

    /** 是否为负时长 */
    bool isNegative() const {
        return m_duration.is_negative();
    }

    /** 转换为 boost::posix_time::time_duration */
    bt::time_duration time_duration() const {
        return m_duration;
    }

    /** 转换为字符串，格式：-1 days hh:mm:ss.000000) */
    std::string str() const {
        return fmt::format("{} days, {:>02d}:{:>02d}:{:>02d}.{:<6d}", days(), hours(), minutes(),
                           seconds(), milliseconds() * 1000 + microseconds());
    }

    /** 转换为字符串，格式为：TimeDelta(days,hours,mins,secs,millisecs,microsecs) */
    std::string repr() const {
        return fmt::format("TimeDelta({}, {}, {}, {}, {}, {})", days(), hours(), minutes(),
                           seconds(), milliseconds(), microseconds());
    }

    //---------------------------------------------------------------
    //
    // 运算符重载
    //
    //---------------------------------------------------------------

    /** 相加 */
    TimeDelta operator+(TimeDelta td) {
        return TimeDelta(td.m_duration + m_duration);
    }

    /** 相减 */
    TimeDelta operator-(TimeDelta td) {
        return TimeDelta(m_duration - td.m_duration);
    }

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

    //---------------------------------------------------------------
    //
    // 静态成员函数
    //
    //---------------------------------------------------------------

    /** 获取能够表达的最小值 TimeDelta(-99999999) */
    static TimeDelta min() {
        return TimeDelta(-99999999);
    }

    /** 获取能够表达的最大值 TimeDelta(99999999, 23, 59, 59, 999, 999) */
    static TimeDelta max() {
        return TimeDelta(99999999, 23, 59, 59, 999, 999);
    }

    /** 获取表达精度 1 微秒, TimeDelta(0, 0, 0, 0, 0, 1) */
    static TimeDelta resolution() {
        return TimeDelta(0, 0, 0, 0, 0, 1);
    }

private:
    bt::time_duration m_duration;

    static constexpr const int64_t m_max_micro_seconds = 100000000LL * 24 * 60 * 60 * 1000000 - 1;
    static constexpr const int64_t m_min_micro_seconds = -99999999LL * 24 * 60 * 60 * 1000000;
};

class HKU_API Days : public TimeDelta {
public:
    Days(int64_t days) : TimeDelta(days) {}
};

class HKU_API Hours : public TimeDelta {
public:
    Hours(int64_t hours) : TimeDelta(0, hours) {}
};

class HKU_API Minutes : public TimeDelta {
public:
    Minutes(int64_t mins) : TimeDelta(0, 0, mins) {}
};

class HKU_API Seconds : public TimeDelta {
public:
    Seconds(int64_t secs) : TimeDelta(0, 0, 0, secs) {}
};

class HKU_API Milliseconds : public TimeDelta {
public:
    Milliseconds(int64_t milliseonds) : TimeDelta(0, 0, 0, 0, milliseonds) {}
};

class HKU_API Microseconds : public TimeDelta {
public:
    Microseconds(int64_t microsecs) : TimeDelta(0, 0, 0, 0, 0, microsecs) {}
};

} /* namespace hku */

#endif /* HIKYUU_DATETIME_TIMEDELTA_H */