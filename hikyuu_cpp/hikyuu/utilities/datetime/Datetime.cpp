/*
 * Datetime.cpp
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <fmt/format.h>
#include "hikyuu/utilities/Null.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/arithmetic.h"
#include "Datetime.h"

namespace hku {

HKU_UTILS_API std::ostream &operator<<(std::ostream &out, const Datetime &d) {
    out << d.str();
    return out;
}

Datetime Datetime::fromHex(uint64_t time) {
    if (Null<unsigned long long>() == time) {
        return Datetime();
    }

    uint64_t second = 0xFFULL & time;
    uint64_t minute = (0xFF00ULL & time) >> 8;
    uint64_t hour = (0xFF0000ULL & time) >> 16;
    uint64_t day = (0xFF000000ULL & time) >> 24;
    uint64_t month = (0xFF00000000ULL & time) >> 32;
    uint64_t low_y = (0xFF0000000000ULL & time) >> 40;
    uint64_t high_y = (0xFF000000000000ULL & time) >> 48;
    uint64_t year = high_y * 100 + low_y;
    return Datetime((long)year, (long)month, (long)day, (long)hour, (long)minute, (long)second);
}

Datetime::Datetime(long year, long month, long day, long hh, long mm, long sec, long millisec,
                   long microsec) {
    HKU_CHECK(millisec >= 0 && millisec <= 999, "Out of range! millisec: {}", millisec);
    HKU_CHECK(microsec >= 0 && microsec <= 999, "Out of range! microsec: {}", microsec);
    bd::date d((unsigned short)year, (unsigned short)month, (unsigned short)day);
    m_data = bt::ptime(d, bt::time_duration(hh, mm, sec, millisec * 1000 + microsec));
}

Datetime::Datetime(unsigned long long datetime) {
    if (Null<unsigned long long>() == datetime) {
        bd::date d(bd::pos_infin);
        m_data = bt::ptime(d, bt::time_duration(0, 0, 0));
        return;
    }

    if (datetime <= 99999999LL) {
        unsigned long long year, month, day;
        year = datetime / 10000;
        month = (datetime - year * 10000) / 100;
        day = datetime - datetime / 100 * 100;
        bd::date d((unsigned short)year, (unsigned short)month, (unsigned short)day);
        m_data = bt::ptime(d, bt::time_duration(0, 0, 0));
    } else if (datetime <= 999999999999LL) {
        unsigned long long year, month, day, hh, mm;
        year = datetime / 100000000;
        month = (datetime - year * 100000000) / 1000000;
        day = (datetime - datetime / 1000000 * 1000000) / 10000;
        hh = (datetime - datetime / 10000 * 10000) / 100;
        mm = (datetime - datetime / 100 * 100);
        HKU_CHECK_THROW(hh < 24, std::out_of_range, "Hour value is out of rang 0..23");
        HKU_CHECK_THROW(mm < 60, std::out_of_range, "Minute value is out of range 0..59");
        bd::date d((unsigned short)year, (unsigned short)month, (unsigned short)day);
        m_data = bt::ptime(d, bt::time_duration((unsigned short)hh, (unsigned short)mm, 0));
    } else if (datetime <= 99999999999999LL) {
        // YYYY MM DD hh mm ss
        unsigned long long year, month, day, hh, mm, ss;
        year = datetime / 10000000000ULL;
        month = (datetime - year * 10000000000ULL) / 100000000ULL;
        day = (datetime - datetime / 100000000ULL * 100000000ULL) / 1000000ULL;
        hh = (datetime - datetime / 1000000ULL * 1000000ULL) / 10000ULL;
        mm = (datetime - datetime / 10000ULL * 10000ULL) / 100ULL;
        ss = (datetime - datetime / 100ULL * 100ULL);
        HKU_CHECK_THROW(hh < 24, std::out_of_range, "Hour value is out of rang 0..23");
        HKU_CHECK_THROW(mm < 60, std::out_of_range, "Minute value is out of range 0..59");
        bd::date d((unsigned short)year, (unsigned short)month, (unsigned short)day);
        m_data = bt::ptime(d, bt::time_duration((unsigned short)hh, (unsigned short)mm, ss));
    } else {
        HKU_THROW_EXCEPTION(
          std::out_of_range,
          "Only support YYYYMMDDhhmmss or YYYYMMDDhhmm or YYYYMMDD, but current param is {}",
          datetime);
    }
}

Datetime::Datetime(const std::string &ts) {
    std::string timeStr(ts);
    trim(timeStr);
    if ("+infinity" == timeStr) {
        m_data = bt::ptime(bd::date(bd::pos_infin), bt::time_duration(0, 0, 0));
    } else if (timeStr.size() <= 10) {
        auto pos1 = timeStr.rfind("-");
        auto pos2 = timeStr.rfind("/");
        m_data = (pos1 != std::string::npos || pos2 != std::string::npos)
                   ? bt::ptime(bd::from_string(timeStr), bt::time_duration(0, 0, 0))
                   : bt::ptime(bd::from_undelimited_string(timeStr), bt::time_duration(0, 0, 0));
    } else {
        to_upper(timeStr);
        auto pos = timeStr.find("T");
        m_data =
          (pos != std::string::npos) ? bt::from_iso_string(timeStr) : bt::time_from_string(timeStr);
    }
}

bool Datetime::isNull() const {
    bd::date d(bd::pos_infin);
    bt::ptime null_date = bt::ptime(d, bt::time_duration(0, 0, 0));
    return (m_data == null_date) ? true : false;
}

Datetime &Datetime::operator=(const Datetime &d) {
    if (this == &d)
        return *this;
    m_data = d.m_data;
    return *this;
}

std::string Datetime::str() const {
    if (isNull()) {
        return "+infinity";
    }

    double microseconds = millisecond() * 1000 + microsecond();

    // 和 python datetime 打印方式保持一致
#if FMT_VERSION >= 90000
    return microseconds == 0
             ? fmt::format("{:>4d}-{:0>2d}-{:0>2d} {:0>2d}:{:0>2d}:{:0>2d}", year(), month(), day(),
                           hour(), minute(), second())
             : fmt::format("{:>4d}-{:0>2d}-{:0>2d} {:0>2d}:{:0>2d}:{:0<9.6f}", year(), month(),
                           day(), hour(), minute(), (second() * 1000000 + microseconds) * 0.000001);
#else
    return microseconds == 0
             ? fmt::format("{:>4d}-{:>02d}-{:>02d} {:>02d}:{:>02d}:{:>02d}", year(), month(), day(),
                           hour(), minute(), second())
             : fmt::format("{:>4d}-{:>02d}-{:>02d} {:>02d}:{:>02d}:{:<09.6f}", year(), month(),
                           day(), hour(), minute(), (second() * 1000000 + microseconds) * 0.000001);
#endif
}

std::string Datetime::repr() const {
    HKU_IF_RETURN(isNull(), "Datetime()");
    return fmt::format("Datetime({},{},{},{},{},{},{},{})", year(), month(), day(), hour(),
                       minute(), second(), millisecond(), microsecond());
}

uint64_t Datetime::number() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<unsigned long long>());
        return (unsigned long long)year() * 100000000ULL +
               (unsigned long long)month() * 1000000ULL + (unsigned long long)day() * 10000ULL +
               (unsigned long long)hour() * 100ULL + (unsigned long long)minute();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

/** 返回如YYYYMM格式的数字*/
uint64_t Datetime::ym() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<unsigned long long>());
        return (unsigned long long)year() * 100ULL + (unsigned long long)month();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

/** 返回如YYYYMMDD格式的数字*/
uint64_t Datetime::ymd() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<unsigned long long>());
        return (unsigned long long)year() * 10000ULL + (unsigned long long)month() * 100ULL +
               (unsigned long long)day();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

/** 返回如YYYYMMDDHH格式的数字*/
uint64_t Datetime::ymdh() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<unsigned long long>());
        return (unsigned long long)year() * 1000000ULL + (unsigned long long)month() * 10000ULL +
               (unsigned long long)day() * 100ULL + (unsigned long long)hour();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

/** 返回如YYYYMMDDhhmm格式的数字*/
uint64_t Datetime::ymdhm() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<unsigned long long>());
        return (unsigned long long)year() * 100000000LL + (unsigned long long)month() * 1000000LL +
               (unsigned long long)day() * 10000LL + (unsigned long long)hour() * 100LL +
               (unsigned long long)minute();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

/** 返回如YYYYMMDDhhmmss格式的数字*/
uint64_t Datetime::ymdhms() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<unsigned long long>());
        return (unsigned long long)year() * 10000000000ULL +
               (unsigned long long)month() * 100000000ULL + (unsigned long long)day() * 1000000ULL +
               (unsigned long long)hour() * 10000ULL + (unsigned long long)minute() * 100ULL +
               (unsigned long long)second();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

uint64_t Datetime::hex() const noexcept {
    try {
        HKU_IF_RETURN(m_data.date() == bd::date(bd::pos_infin), Null<unsigned long long>());
        uint64_t ret = uint64_t(second());
        ret |= (uint64_t(minute()) << 8);
        ret |= (uint64_t(hour()) << 16);
        ret |= (uint64_t(day()) << 24);
        ret |= (uint64_t(month()) << 32);
        uint64_t y = uint64_t(year());
        uint64_t high_y = y / 100;
        uint64_t low_y = y - high_y * 100;
        ret |= (low_y << 40);
        ret |= (high_y << 48);
        return ret;
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

uint64_t Datetime::ticks() const noexcept {
    try {
        HKU_IF_RETURN(isNull(), Null<uint64_t>());
        TimeDelta d = (*this) - Datetime::min();
        return d.ticks();
    } catch (const std::exception &e) {
        HKU_ERROR(e.what());
        return Null<uint64_t>();
    } catch (...) {
        HKU_ERROR("Unknown error!");
        return Null<uint64_t>();
    }
}

long Datetime::year() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return m_data.date().year();
}

long Datetime::month() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return m_data.date().month();
}

long Datetime::day() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return m_data.date().day();
}

long Datetime::hour() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return long(m_data.time_of_day().hours());
}

long Datetime::minute() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return long(m_data.time_of_day().minutes());
}

long Datetime::second() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return long(m_data.time_of_day().seconds());
}

long Datetime::millisecond() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return long(m_data.time_of_day().fractional_seconds()) / 1000;
}

long Datetime::microsecond() const {
    HKU_CHECK_THROW(!isNull(), std::logic_error, "This is Null Datetime!");
    return long(m_data.time_of_day().fractional_seconds()) % 1000;
}

Datetime Datetime::min() {
    bd::date d(bd::min_date_time);
    return Datetime(d.year(), d.month(), d.day());
}

Datetime Datetime::max() {
    bd::date d(bd::max_date_time);
    return Datetime(d.year(), d.month(), d.day());
}

Datetime Datetime::now() {
    return Datetime(bt::microsec_clock::local_time());
}

Datetime Datetime::today() {
    Datetime x = Datetime::now();
    return Datetime(x.year(), x.month(), x.day());
}

DatetimeList HKU_UTILS_API getDateRange(const Datetime &start, const Datetime &end) {
    DatetimeList result;
    bd::date start_day = start.date();
    bd::date end_day = end.date();
    bd::date_period dp(start_day, end_day);
    bd::day_iterator iter = dp.begin();
    for (; iter != dp.end(); ++iter) {
        result.push_back(Datetime(*iter));
    }
    return result;
}

Datetime Datetime::dateOfWeek(int day) const {
    HKU_IF_RETURN(*this == Null<Datetime>(), *this);

    int dd = day;
    if (dd < 0) {
        dd = 0;
    } else if (dd > 6) {
        dd = 6;
    }
    int today = dayOfWeek();
    Datetime result(date() + bd::date_duration(dd - today));
    if (result > Datetime::max()) {
        result = Datetime::max();
    } else if (result < Datetime::min()) {
        result = Datetime::min();
    }
    return result;
}

Datetime Datetime::startOfMonth() const {
    return *this == Null<Datetime>() ? *this : Datetime(year(), month(), 1);
}

Datetime Datetime::endOfMonth() const {
    return *this == Null<Datetime>() ? *this : Datetime(date().end_of_month());
}

Datetime Datetime::startOfYear() const {
    return *this == Null<Datetime>() ? *this : Datetime(year(), 1, 1);
}

Datetime Datetime::endOfYear() const {
    return *this == Null<Datetime>() ? Null<Datetime>() : Datetime(year(), 12, 31);
}

Datetime Datetime::startOfWeek() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    int today = dayOfWeek();
    if (today == 0) {
        result = Datetime(date() + bd::date_duration(-6));
    } else {
        result = Datetime(date() + bd::date_duration(1 - today));
        ;
    }

    if (result < Datetime::min())
        result = Datetime::min();

    return result;
}

Datetime Datetime::endOfWeek() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    int today = dayOfWeek();
    if (today == 0) {
        result = Datetime(date());
    } else {
        result = Datetime(date() + bd::date_duration(7 - today));
    }

    if (result > Datetime::max())
        result = Datetime::max();
    return result;
}

Datetime Datetime::startOfQuarter() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    int m = month();
    int y = year();
    if (m <= 3) {
        result = Datetime(y, 1, 1);
    } else if (m <= 6) {
        result = Datetime(y, 4, 1);
    } else if (m <= 9) {
        result = Datetime(y, 7, 1);
    } else if (m <= 12) {
        result = Datetime(y, 10, 1);
    }

    return result;
}

Datetime Datetime::endOfQuarter() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    int m = month();
    int y = year();
    if (m <= 3) {
        result = Datetime(y, 3, 31);
    } else if (m <= 6) {
        result = Datetime(y, 6, 30);
    } else if (m <= 9) {
        result = Datetime(y, 9, 30);
    } else if (m <= 12) {
        result = Datetime(y, 12, 31);
    }

    return result;
}

Datetime Datetime::startOfHalfyear() const {
    HKU_IF_RETURN(*this == Null<Datetime>(), *this);
    return month() <= 6 ? Datetime(year(), 1, 1) : Datetime(year(), 7, 1);
}

Datetime Datetime::endOfHalfyear() const {
    HKU_IF_RETURN(*this == Null<Datetime>(), *this);
    return month() <= 6 ? Datetime(year(), 6, 30) : Datetime(year(), 12, 31);
}

Datetime Datetime::nextDay() const {
    HKU_IF_RETURN(*this == Null<Datetime>() || *this == Datetime::max(), *this);
    return Datetime(date() + bd::date_duration(1));
}

Datetime Datetime::nextWeek() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    result = Datetime(endOfWeek().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextMonth() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    result = Datetime(endOfMonth().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextQuarter() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    result = Datetime(endOfQuarter().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextHalfyear() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    result = Datetime(endOfHalfyear().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextYear() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    result = Datetime(endOfYear().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();
    return result;
}

Datetime Datetime::preDay() const {
    HKU_IF_RETURN(*this == Null<Datetime>() || *this == Datetime::min(), *this);
    return Datetime(date() - bd::date_duration(1));
}

Datetime Datetime::preWeek() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    try {
        result = Datetime(date() - bd::date_duration(7)).startOfWeek();
    } catch (...) {
        result = Datetime::min();
    }
    return result;
}

Datetime Datetime::preMonth() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    try {
        int m = month();
        result = (m == 1) ? Datetime(year() - 1, 12, 1) : Datetime(year(), m - 1, 1);
    } catch (...) {
        result = Datetime::min();
    }
    return result;
}

Datetime Datetime::preQuarter() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    try {
        int m = startOfQuarter().month();
        result = (m == 1) ? Datetime(year() - 1, 10, 1) : Datetime(year(), m - 3, 1);
    } catch (...) {
        result = Datetime::min();
    }

    return result;
}

Datetime Datetime::preHalfyear() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    try {
        int m = startOfHalfyear().month();
        result = (m <= 6) ? Datetime(year() - 1, 7, 1) : Datetime(year(), 1, 1);
    } catch (...) {
        result = Datetime::min();
    }

    return result;
}

Datetime Datetime::preYear() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    try {
        result = Datetime(year() - 1, 1, 1);
    } catch (...) {
        result = Datetime::min();
    }

    return result;
}

Datetime Datetime::endOfDay() const {
    Datetime result;
    HKU_IF_RETURN(*this == Null<Datetime>(), result);
    result = date() != bd::date(bd::max_date_time) ? Datetime(year(), month(), day(), 23, 59, 59)
                                                   : Datetime::max();
    return result;
}

} /* namespace hku */
