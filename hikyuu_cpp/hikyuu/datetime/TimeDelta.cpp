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
#include "../utilities/exception.h"

namespace hku {

TimeDelta::TimeDelta(int64_t days, int64_t hours, int64_t minutes, int64_t seconds,
                     int64_t milliseconds, int64_t microseconds) {
    HKU_CHECK(days <= 99999999 && days >= -99999999, "Out of range! Input days: {}", days);
    HKU_CHECK(hours >= -10000 && hours <= 10000, "Out of range! Input hours: {}", hours);
    HKU_CHECK(minutes >= -50000 && minutes <= 50000, "Out of range! Input minutes: {}", minutes);
    HKU_CHECK(seconds >= -86399 && seconds <= 86399, "Out of range! Input seconds: {}", seconds);
    HKU_CHECK(milliseconds >= -999 && milliseconds <= 999, "Out of range! Input milliseconds: {}",
              milliseconds);
    HKU_CHECK(microseconds >= -999 && microseconds <= 999, "Out of range! Input microseconds: {}",
              microseconds);
    int64_t total =
      ((((days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000 + milliseconds) * 1000 +
      microseconds;
    if (total < 0) {
        total = -(86400000000LL + total);
    }
    HKU_CHECK(total >= m_min_micro_seconds && total <= m_max_micro_seconds, "Out of total range!");
    m_duration = bt::time_duration(0, 0, 0, total);
}

TimeDelta::TimeDelta(bt::time_duration td) {
    int64_t total = td.total_microseconds();
    HKU_CHECK(total >= m_min_micro_seconds && total <= m_max_micro_seconds, "Out of total range!");
    m_duration = td;
}

} /* namespace hku */