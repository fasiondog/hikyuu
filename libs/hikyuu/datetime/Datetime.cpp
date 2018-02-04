/*
 * Datetime.cpp
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#include "../utilities/Null.h"
#include "Datetime.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream &out, const Datetime& d) {
    if (d == Null<Datetime>()) {
        out << "+infinity";
    } else {
        out << d.year() << "-" << d.month() << "-" << d.day() << " "
            << d.hour() << ":" << d.minute() << ":" << d.second();
    }
    return out;
}

Datetime::Datetime(unsigned long long datetime) {
    if(Null<unsigned long long>() == datetime) {
        bd::date d(bd::pos_infin);
        m_data = bt::ptime(d, bt::time_duration(0,0,0));
        return;
    }

    unsigned long long year, month, day, hh, mm;
    year = datetime / 100000000;
    month  = (datetime - year * 100000000) / 1000000;
    day  = (datetime - datetime / 1000000 * 1000000) / 10000;
    hh = (datetime - datetime / 10000 * 10000) / 100;
    mm = (datetime - datetime / 100 * 100);
    bd::date d((unsigned short)year,
               (unsigned short)month,
               (unsigned short)day);
    if(hh >=24 ) {
        throw std::out_of_range("Hour value is out of rang 0..23");
    }
    if(mm >= 60) {
        throw std::out_of_range("Minute value is out of range 0..59");
    }
    m_data = bt::ptime(d, bt::time_duration((unsigned short)hh,
                                            (unsigned short)mm, 0));
}


std::string Datetime::toString() const {
    bd::date d(bd::pos_infin);
    bt::ptime null_date = bt::ptime(d, bt::time_duration(0,0,0));

    if (m_data == null_date) {
        return "+infinity";
    }

    //不能像operator << 只输出到分钟信息，会导致序列化读取后不等
    return bt::to_simple_string(m_data);
}

unsigned long long Datetime::number() const {
    if(m_data.date() == bd::date(bd::pos_infin)) {
        return Null<unsigned long long>();
    }

    return (unsigned long long)year() * 100000000
            + (unsigned long long)month() * 1000000
            + (unsigned long long)day() * 10000
            + (unsigned long long)hour() * 100
            + (unsigned long long)minute();
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

DatetimeList HKU_API getDateRange(const Datetime& start, const Datetime& end) {
    DatetimeList result;
    bd::date start_day = start.date();
    bd::date end_day = end.date();
    bd::date_period dp(start_day, end_day);
    bd::day_iterator iter = dp.begin();
    for (; iter != dp.end(); ++iter) {
        result.push_back(*iter);
    }
    return result;
}

Datetime Datetime::nextDay() const {
    bd::date today = date();
    bd::date_duration dd(1);
    bd::date next = today + dd;
    return Datetime(next);
}

} /* namespace hku */
