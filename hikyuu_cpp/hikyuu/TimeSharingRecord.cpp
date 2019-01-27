/*
 * TimeSharingRecord.cpp
 *
 *  Created on: 2019年1月27日
 *      Author: fasiondog
 */

#include "TimeSharingRecord.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const TimeSharingRecord& record) {
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

bool HKU_API operator==(const TimeSharingRecord& d1, const TimeSharingRecord&d2) {
    if (d1.datetime == d2.datetime
            &&  (std::fabs(d1.price - d2.price) < 0.0001)
            &&  (std::fabs(d1.vol - d2.vol) < 0.0001)) {
        return true;
    }

    return false;
}


TimeSharingRecord::TimeSharingRecord()
: datetime(Datetime()), price(0.0), vol(0.0) {

}

TimeSharingRecord::TimeSharingRecord(Datetime datetime, price_t price, price_t vol)
: datetime(datetime), price(price), vol(vol) {

}

TimeSharingRecord::~TimeSharingRecord() {

}

bool TimeSharingRecord::isValid() const {
    return datetime == Null<Datetime>() ? false : true;
}




} /* namespace hku */
