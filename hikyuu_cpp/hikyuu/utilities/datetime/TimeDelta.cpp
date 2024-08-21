/*
 * TimeDelta.h
 *
 *  Copyright(C) 2019, hikyuu.org
 *
 *  Created on: 2019-12-12
 *      Author: fasiondog
 */

#include <cstdint>
#include "TimeDelta.h"
#include "hikyuu/utilities/arithmetic.h"
#include "hikyuu/utilities/Log.h"

namespace hku {

TimeDelta::TimeDelta(int64_t days, int64_t hours, int64_t minutes, int64_t seconds,
                     int64_t milliseconds, int64_t microseconds) {
    // 各参数添加限制，防止出现总和溢出的情况
    HKU_CHECK(days <= 99999999 && days >= -99999999, "Out of range! Input days: {}", days);
    HKU_CHECK(hours >= -100000 && hours <= 100000, "Out of range! Input hours: {}", hours);
    HKU_CHECK(minutes >= -100000 && minutes <= 100000, "Out of range! Input minutes: {}", minutes);
    HKU_CHECK(seconds >= -8639900 && seconds <= 8639900, "Out of range! Input seconds: {}",
              seconds);
    HKU_CHECK(milliseconds >= -86399000000 && milliseconds <= 86399000000,
              "Out of range! Input milliseconds: {}", milliseconds);
    HKU_CHECK(microseconds >= -86399000000 && microseconds <= 86399000000,
              "Out of range! Input microseconds: {}", microseconds);
    int64_t total =
      ((((days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000 + milliseconds) * 1000 +
      microseconds;
    HKU_CHECK(total >= m_min_micro_seconds && total <= m_max_micro_seconds, "Out of total range!");
    m_duration = bt::time_duration(0, 0, 0, total);
}

TimeDelta::TimeDelta(bt::time_duration td) {
    int64_t total = td.total_microseconds();
    HKU_CHECK(total >= m_min_micro_seconds && total <= m_max_micro_seconds, "Out of total range!");
    m_duration = td;
}

/** 从字符串构造，格式：-1 days, hh:mm:ss.000000) */
TimeDelta::TimeDelta(const std::string& delta) {
    std::string val(delta);
    std::string errmsg(fmt::format("Invalid format: {}", delta));
    to_lower(val);
    auto vals = split(val, ' ');
    HKU_CHECK(vals.size() == 3, errmsg);
    int64_t days = std::stoll(std::string(vals[0]));
    vals = split(vals[2], ':');
    HKU_CHECK(vals.size() == 3, errmsg);
    int64_t hours = std::stoll(std::string(vals[0]));
    int64_t minutes = std::stoll(std::string(vals[1]));
    int64_t microseconds = static_cast<int64_t>(std::stod(std::string(vals[2])) * 1000000.0);
    int64_t total = (((days * 24) + hours) * 60 + minutes) * 60000000LL + microseconds;
    HKU_CHECK(total >= m_min_micro_seconds && total <= m_max_micro_seconds, "Out of total range!");
    m_duration = bt::time_duration(0, 0, 0, total);
}

TimeDelta TimeDelta::fromTicks(int64_t ticks) {
    HKU_CHECK(ticks >= m_min_micro_seconds && ticks <= m_max_micro_seconds, "Out of total range!");
    return TimeDelta(bt::time_duration(0, 0, 0, ticks));
}

int64_t TimeDelta::days() const {
    if (isNegative()) {
        if (ticks() % m_one_day_ticks == 0) {
            return ticks() / m_one_day_ticks;
        } else {
            return ticks() / m_one_day_ticks - 1;
        }
    }
    return std::abs(m_duration.hours() / 24);
}

int64_t TimeDelta::hours() const {
    if (isNegative()) {
        if (ticks() % m_one_day_ticks == 0) {
            return 0;
        } else {
            int64_t pos_ticks =
              std::abs((ticks() / m_one_day_ticks - 1) * m_one_day_ticks) + ticks();
            return bt::time_duration(0, 0, 0, pos_ticks).hours();
        }
    }
    return std::abs(m_duration.hours()) % 24;
}

int64_t TimeDelta::minutes() const {
    if (isNegative()) {
        if (ticks() % m_one_day_ticks == 0) {
            return 0;
        } else {
            int64_t pos_ticks =
              std::abs((ticks() / m_one_day_ticks - 1) * m_one_day_ticks) + ticks();
            return bt::time_duration(0, 0, 0, pos_ticks).minutes();
        }
    }
    return std::abs(m_duration.minutes());
}

int64_t TimeDelta::seconds() const {
    if (isNegative()) {
        if (ticks() % m_one_day_ticks == 0) {
            return 0;
        } else {
            int64_t pos_ticks =
              std::abs((ticks() / m_one_day_ticks - 1) * m_one_day_ticks) + ticks();
            return bt::time_duration(0, 0, 0, pos_ticks).seconds();
        }
    }
    return std::abs(m_duration.seconds());
}

int64_t TimeDelta::milliseconds() const {
    if (isNegative()) {
        if (ticks() % m_one_day_ticks == 0) {
            return 0;
        } else {
            int64_t pos_ticks =
              std::abs((ticks() / m_one_day_ticks - 1) * m_one_day_ticks) + ticks();
            int64_t milli = pos_ticks % 1000000;
            return milli == 0 ? 0 : (milli - microseconds()) / 1000;
        }
    }
    return (std::abs(ticks()) % 1000000 - microseconds()) / 1000;
}

int64_t TimeDelta::microseconds() const {
    if (isNegative()) {
        if (ticks() % m_one_day_ticks == 0) {
            return 0;
        } else {
            int64_t micro = ticks() % 1000;
            return micro == 0 ? micro : 1000 + micro;
        }
    }
    return std::abs(ticks() % 1000);
}

TimeDelta HKU_UTILS_API Hours(int64_t hours) {
    HKU_CHECK(hours >= TimeDelta::minTicks() / 3600000000LL &&
                hours <= TimeDelta::maxTicks() / 3600000000LL,
              "Out of total range!");
    return TimeDelta::fromTicks(hours * 3600000000LL);
}

TimeDelta HKU_UTILS_API Minutes(int64_t mins) {
    HKU_CHECK(
      mins >= TimeDelta::minTicks() / 60000000LL && mins <= TimeDelta::maxTicks() / 60000000LL,
      "Out of total range!");
    return TimeDelta::fromTicks(mins * 60000000LL);
}

TimeDelta HKU_UTILS_API Seconds(int64_t secs) {
    HKU_CHECK(
      secs >= TimeDelta::minTicks() / 1000000LL && secs <= TimeDelta::maxTicks() / 1000000LL,
      "Out of total range!");
    return TimeDelta::fromTicks(secs * 1000000LL);
}

TimeDelta HKU_UTILS_API Milliseconds(int64_t milliseconds) {
    HKU_CHECK(milliseconds >= TimeDelta::minTicks() / 1000LL &&
                milliseconds <= TimeDelta::maxTicks() / 1000LL,
              "Out of total range!");
    return TimeDelta::fromTicks(milliseconds * 1000LL);
}

TimeDelta TimeDelta::operator*(double p) const {
    return TimeDelta::fromTicks(static_cast<int64_t>(roundEx<double>(double(ticks()) * p, 0)));
}

TimeDelta TimeDelta::operator/(double p) const {
    HKU_CHECK(p != 0, "Attempt to divide by 0!");
    return TimeDelta::fromTicks(static_cast<int64_t>(roundEx<double>(double(ticks()) / p, 0)));
}

TimeDelta TimeDelta::floorDiv(double p) const {
    HKU_CHECK(p != 0, "Attempt to divide by 0!");
    return TimeDelta::fromTicks(static_cast<int64_t>(double(ticks()) / p));
}

double TimeDelta::operator/(TimeDelta td) const {
    HKU_CHECK(td.ticks() != 0, "Attemp to divide by zero TimeDelta!");
    return double(ticks()) / double(td.ticks());
}

TimeDelta TimeDelta::operator%(TimeDelta td) const {
    HKU_CHECK(td.ticks() != 0, "Attemp to divide(mod) by zero TimeDelta!");
    return TimeDelta::fromTicks(ticks() % td.ticks());
}

} /* namespace hku */