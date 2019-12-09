/*
 * Datetime.cpp
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#include <fmt/format.h>
#include "../utilities/Null.h"
#include "Datetime.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const Datetime& d) {
    out << d.toString();
    return out;
}

Datetime::Datetime(unsigned long long datetime) {
    if (Null<unsigned long long>() == datetime) {
        bd::date d(bd::pos_infin);
        m_data = bt::ptime(d, bt::time_duration(0, 0, 0));
        return;
    }

    unsigned long long year, month, day, hh, mm;
    year = datetime / 100000000;
    month = (datetime - year * 100000000) / 1000000;
    day = (datetime - datetime / 1000000 * 1000000) / 10000;
    hh = (datetime - datetime / 10000 * 10000) / 100;
    mm = (datetime - datetime / 100 * 100);
    bd::date d((unsigned short)year, (unsigned short)month, (unsigned short)day);
    if (hh >= 24) {
        throw std::out_of_range("Hour value is out of rang 0..23");
    }
    if (mm >= 60) {
        throw std::out_of_range("Minute value is out of range 0..59");
    }
    m_data = bt::ptime(d, bt::time_duration((unsigned short)hh, (unsigned short)mm, 0));
}

Datetime& Datetime::operator=(const Datetime& d) {
    if (this == &d)
        return *this;
    m_data = d.m_data;
    return *this;
}

std::string Datetime::toString() const {
    bd::date d(bd::pos_infin);
    bt::ptime null_date = bt::ptime(d, bt::time_duration(0, 0, 0));

    if (m_data == null_date) {
        return "+infinity";
    }

    std::string result;
    if (microsecond() == 0) {
        result = fmt::format("{:>4d}-{:>02d}-{:>02d} {:>02d}:{:>02d}:{:>02d}", year(), month(),
                             day(), hour(), minute(), second());
    } else {
        result =
          fmt::format("{:>4d}-{:>02d}-{:>02d} {:>02d}:{:>02d}:{:<09.6f}", year(), month(), day(),
                      hour(), minute(), double(second()) + double(microsecond()) * 0.000001);
    }
    return result;
    // return bt::to_simple_string(m_data);
}

unsigned long long Datetime::number() const {
    if (m_data.date() == bd::date(bd::pos_infin)) {
        return Null<unsigned long long>();
    }

    return (unsigned long long)year() * 100000000 + (unsigned long long)month() * 1000000 +
           (unsigned long long)day() * 10000 + (unsigned long long)hour() * 100 +
           (unsigned long long)minute();
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

DatetimeList HKU_API getDateRange(const Datetime& start, const Datetime& end) {
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
    if (*this == Null<Datetime>())
        return *this;

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
    if (*this == Null<Datetime>())
        return *this;

    Datetime result;
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
    if (*this == Null<Datetime>())
        return *this;

    Datetime result;
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
    if (*this == Null<Datetime>())
        return result;

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
    if (*this == Null<Datetime>())
        return result;

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
    if (*this == Null<Datetime>())
        return *this;

    return month() <= 6 ? Datetime(year(), 1, 1) : Datetime(year(), 7, 1);
}

Datetime Datetime::endOfHalfyear() const {
    if (*this == Null<Datetime>())
        return *this;

    return month() <= 6 ? Datetime(year(), 6, 30) : Datetime(year(), 12, 31);
}

Datetime Datetime::nextDay() const {
    if (*this == Null<Datetime>() || *this == Datetime::max())
        return *this;
    return Datetime(date() + bd::date_duration(1));
}

Datetime Datetime::nextWeek() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

    result = Datetime(endOfWeek().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextMonth() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

    result = Datetime(endOfMonth().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextQuarter() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

    result = Datetime(endOfQuarter().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextHalfyear() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

    result = Datetime(endOfHalfyear().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();

    return result;
}

Datetime Datetime::nextYear() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

    result = Datetime(endOfYear().date() + bd::date_duration(1));
    if (result > Datetime::max())
        result = Datetime::max();
    return result;
}

Datetime Datetime::preDay() const {
    if (*this == Null<Datetime>() || *this == Datetime::min())
        return *this;
    return Datetime(date() - bd::date_duration(1));
}

Datetime Datetime::preWeek() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

    try {
        result = Datetime(date() - bd::date_duration(7)).startOfWeek();
    } catch (...) {
        result = Datetime::min();
    }
    return result;
}

Datetime Datetime::preMonth() const {
    Datetime result;
    if (*this == Null<Datetime>())
        return result;

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
    if (*this == Null<Datetime>())
        return result;

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
    if (*this == Null<Datetime>())
        return result;

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
    if (*this == Null<Datetime>())
        return result;

    try {
        result = Datetime(year() - 1, 1, 1);
    } catch (...) {
        result = Datetime::min();
    }

    return result;
}

Datetime Datetime::endOfDay() const {
    Datetime result;
    if (*this == Null<Datetime>()) {
        return result;
    }

    result = date() != bd::date(bd::max_date_time) ? Datetime(year(), month(), day(), 23, 59, 59)
                                                   : Datetime::max();
    return result;
}

} /* namespace hku */
