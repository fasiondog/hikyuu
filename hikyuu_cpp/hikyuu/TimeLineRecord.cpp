/*
 * TimeLineRecord.cpp
 *
 *  Created on: 2019年1月27日
 *      Author: fasiondog
 */

#include "TimeLineRecord.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const TimeLineRecord& record) {
    string strip(", ");
    os << std::fixed;
    os.precision(4);
    os << "TimeSharingRecord(Datetime(" << record.datetime.number() << ")"<< strip
            << record.price << strip
            << record.vol << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

bool HKU_API operator==(const TimeLineRecord& d1, const TimeLineRecord&d2) {
    if (d1.datetime == d2.datetime
            &&  (std::fabs(d1.price - d2.price) < 0.0001)
            &&  (std::fabs(d1.vol - d2.vol) < 0.0001)) {
        return true;
    }

    return false;
}


TimeLineRecord::TimeLineRecord()
: datetime(Datetime()), price(0.0), vol(0.0) {

}

TimeLineRecord::TimeLineRecord(const Datetime& datetime,
        price_t price, price_t vol)
: datetime(datetime), price(price), vol(vol) {

}

TimeLineRecord::~TimeLineRecord() {

}

bool TimeLineRecord::isValid() const {
    return datetime == Null<Datetime>() ? false : true;
}


} /* namespace hku */
