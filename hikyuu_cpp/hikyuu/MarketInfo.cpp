/*
 * MarketInfo.cpp
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketInfo.h"

namespace hku {

namespace {

/** The nth Sunday of the given month, at midnight */
Datetime nthSunday(long year, long month, long n) {
    long first_dow = Datetime(year, month, 1).dayOfWeek();
    return Datetime(year, month, 1 + (7 - first_dow) % 7 + (n - 1) * 7);
}

/** The last Sunday of the given month, at midnight */
Datetime lastSunday(long year, long month) {
    long last_day = bd::date(year, month, 1).end_of_month().day();
    return Datetime(year, month, last_day - Datetime(year, month, last_day).dayOfWeek());
}

/** The minutes of the given duration inside a day */
int minutesOfDay(const TimeDelta& td) {
    return int(td.total_seconds() / 60);
}

/**
 * Whether the session [open,close) overlaps [lo,hi); an empty or reversed session covers nothing
 */
bool sessionIntersect(int open_min, int close_min, int lo, int hi) {
    if (open_min >= close_min) {
        return false;
    }
    return open_min < hi && lo < close_min;
}

}  // namespace

bool MarketInfo::hasAmbiguousSession() const {
    // Same as isTradingHours: both session pairs equal means 24h; a single 0/0 pair is unset
    if (m_openTime1 == m_closeTime1 && m_openTime2 == m_closeTime2) {
        return true;
    }
    // The local hour that the rule's fall-back repeats: the US rule switches at 02:00 local
    // (repeating [01:00,02:00)), the EU rule switches at 01:00 UTC (repeating [02:00,03:00))
    int lo, hi;
    switch (m_dstRule) {
        case 1:
            lo = 60;
            hi = 120;
            break;
        case 2:
            lo = 120;
            hi = 180;
            break;
        default:
            return false;
    }
    int o1(minutesOfDay(m_openTime1)), c1(minutesOfDay(m_closeTime1));
    int o2(minutesOfDay(m_openTime2)), c2(minutesOfDay(m_closeTime2));
    return sessionIntersect(o1, c1, lo, hi) || sessionIntersect(o2, c2, lo, hi);
}

bool MarketInfo::_isDstUTC(const Datetime& utc) const {
    HKU_IF_RETURN(m_dstRule == 0 || utc.isNull(), false);

    long y = utc.year();
    Datetime start, end;
    switch (m_dstRule) {
        case 1:
            // US: 2nd Sun Mar - 1st Sun Nov since 2007; before that 1st Sun Apr - last Sun Oct
            if (y >= 2007) {
                start = nthSunday(y, 3, 2);
                end = nthSunday(y, 11, 1);
            } else {
                start = nthSunday(y, 4, 1);
                end = lastSunday(y, 10);
            }
            // Both switch at 02:00 local (start standard, end daylight), hence different instants
            start = start - Minutes(m_tzOffsetMin) + Hours(2);
            end = end - Minutes(m_tzOffsetMin + 60) + Hours(2);
            break;
        case 2:
            // EU: the last Sundays of March and October, both switching at 01:00 UTC
            start = lastSunday(y, 3) + Hours(1);
            end = lastSunday(y, 10) + Hours(1);
            break;
        default:
            return false;
    }
    return utc >= start && utc < end;
}

Datetime MarketInfo::localToUnified(const Datetime& local) const {
    HKU_IF_RETURN(local.isNull(), local);
    // Guess with the standard offset; landing inside daylight means the daylight offset applies
    Datetime utc = local - Minutes(m_tzOffsetMin);
    if (_isDstUTC(utc)) {
        utc = local - Minutes(m_tzOffsetMin + 60);
    }
    return utc + Minutes(UNIFIED_TZ_OFFSET_MIN);
}

Datetime MarketInfo::unifiedToLocal(const Datetime& unified) const {
    HKU_IF_RETURN(unified.isNull(), unified);
    // UTC carries no ambiguity, so the daylight state can be read off directly
    Datetime utc = unified - Minutes(UNIFIED_TZ_OFFSET_MIN);
    int offset(m_tzOffsetMin + (_isDstUTC(utc) ? 60 : 0));
    return utc + Minutes(offset);
}

HKU_API std::ostream& operator<<(std::ostream& os, const MarketInfo& market) {
    os << market.toString();
    return os;
}

string MarketInfo::toString() const {
    std::stringstream os;
    if (m_market == "") {
        os << "MarketInfo()";
        return os.str();
    }

    string sp(", ");
    os << "MarketInfo(" << m_market << sp << m_name << sp << m_description << sp << m_code << sp
       << m_lastDate << sp << m_openTime1.hours() << ":" << m_openTime1.minutes() << sp
       << m_closeTime1.hours() << ":" << m_closeTime1.minutes() << sp << m_openTime2.hours() << ":"
       << m_openTime2.minutes() << sp << m_closeTime2.hours() << ":" << m_closeTime2.minutes() << sp
       << m_tzOffsetMin << sp << m_dstRule << sp << m_dataLag << ")";
    return os.str();
}

bool MarketInfo::equals(const MarketInfo& rhs) const {
    return m_market == rhs.m_market && m_name == rhs.m_name && m_description == rhs.m_description &&
           m_code == rhs.m_code && m_lastDate == rhs.m_lastDate && m_openTime1 == rhs.m_openTime1 &&
           m_closeTime1 == rhs.m_closeTime1 && m_openTime2 == rhs.m_openTime2 &&
           m_closeTime2 == rhs.m_closeTime2 && m_tzOffsetMin == rhs.m_tzOffsetMin &&
           m_dstRule == rhs.m_dstRule && m_dataLag == rhs.m_dataLag;
}

MarketInfo::MarketInfo() {}

MarketInfo::MarketInfo(const string& market, const string& name, const string& description,
                       const string& code, const Datetime& lastDate, TimeDelta openTime1,
                       TimeDelta closeTime1, TimeDelta openTime2, TimeDelta closeTime2,
                       int tzOffsetMin, int dstRule, int dataLag)
: m_market(market),
  m_name(name),
  m_description(description),
  m_code(code),
  m_lastDate(lastDate),
  m_openTime1(openTime1),
  m_closeTime1(closeTime1),
  m_openTime2(openTime2),
  m_closeTime2(closeTime2),
  m_tzOffsetMin(tzOffsetMin),
  m_dstRule(dstRule),
  m_dataLag(dataLag) {
    // Sessions reaching the ambiguous hour can not be converted back; reject at construction
    HKU_CHECK(dstRule == 0 || !hasAmbiguousSession(),
              "The trading sessions of market [{}] intersect the repeated local hour of its "
              "daylight rule [{}]!",
              market, dstRule);
}

MarketInfo::MarketInfo(MarketInfo&& rhs) noexcept
: m_market(std::move(rhs.m_market)),
  m_name(std::move(rhs.m_name)),
  m_description(std::move(rhs.m_description)),
  m_code(std::move(rhs.m_code)),
  m_lastDate(std::move(rhs.m_lastDate)),
  m_openTime1(std::move(rhs.m_openTime1)),
  m_closeTime1(std::move(rhs.m_closeTime1)),
  m_openTime2(std::move(rhs.m_openTime2)),
  m_closeTime2(std::move(rhs.m_closeTime2)),
  m_tzOffsetMin(rhs.m_tzOffsetMin),
  m_dstRule(rhs.m_dstRule),
  m_dataLag(rhs.m_dataLag) {}

MarketInfo& MarketInfo::operator=(MarketInfo&& rhs) noexcept {
    if (this != &rhs) {
        m_market = std::move(rhs.m_market);
        m_name = std::move(rhs.m_name);
        m_description = std::move(rhs.m_description);
        m_code = std::move(rhs.m_code);
        m_lastDate = std::move(rhs.m_lastDate);
        m_openTime1 = std::move(rhs.m_openTime1);
        m_closeTime1 = std::move(rhs.m_closeTime1);
        m_openTime2 = std::move(rhs.m_openTime2);
        m_closeTime2 = std::move(rhs.m_closeTime2);
        m_tzOffsetMin = rhs.m_tzOffsetMin;
        m_dstRule = rhs.m_dstRule;
        m_dataLag = rhs.m_dataLag;
    }
    return *this;
}

}  // namespace hku
