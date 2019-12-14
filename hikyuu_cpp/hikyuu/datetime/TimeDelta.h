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
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

/**
 * 持续时间，用于时间计算
 * @ingroup DataType
 */
class HKU_API TimeDelta {
public:
    /**
     * 构造函数
     * @param days 天数 [-99999999, 99999999]
     * @param hours 小时数 [0, 10000]
     * @param minutes 分钟数 [0, 50000]
     * @param seconds 秒数 [0, 86399]
     * @param milliseconds 毫秒数 [0, 999]
     * @param microseconds 微秒数 [0, 999]
     */
    TimeDelta(int64_t days = 0, int64_t hours = 0, int64_t minutes = 0, int64_t seconds = 0,
              int64_t milliseconds = 0, int64_t microseconds = 0);

    explicit TimeDelta(bt::time_duration td);

    int64_t days() const {
        return m_duration.hours() / 24;
    }

    int64_t hours() const {
        return std::abs(m_duration.hours() - days() * 24);
    }

    int64_t minutes() const {
        return m_duration.minutes();
    }

    int64_t seconds() const {
        return m_duration.seconds();
    }

    int64_t milliseconds() const {
        return ticks() % 1000000 - microseconds();
    }

    int64_t microseconds() const {
        return ticks() % 1000;
    }

    int64_t ticks() const {
        return m_duration.ticks();
    }

    bool isNegative() const {
        return m_duration.is_negative();
    }

    bt::time_duration time_duration() const {
        return m_duration;
    }

    std::string str() const;

    static TimeDelta min() {
        return TimeDelta(-99999999);
    }

    static TimeDelta max() {
        return TimeDelta(99999999, 23, 59, 59, 999, 999);
    }

    static TimeDelta resolution() {
        return TimeDelta(0, 0, 0, 0, 0, 1);
    }

private:
    bt::time_duration m_duration;

    static constexpr const int64_t m_max_micro_seconds = 10000LL * 24 * 60 * 60 * 1000000 - 1;
    static constexpr const int64_t m_min_micro_seconds = -9999LL * 24 * 60 * 60 * 1000000;
};

std::ostream& operator<<(std::ostream&, const TimeDelta&);
inline std::ostream& operator<<(std::ostream& out, const TimeDelta& td) {
    out << td.str();
    return out;
}

} /* namespace hku */

#endif /* HIKYUU_DATETIME_TIMEDELTA_H */