/*
 * TransRecord.cpp
 *
 *  Created on: 2019年2月10日
 *      Author: fasiondog
 */

#include "TransRecord.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const TransRecord& record) {
    string strip(", ");
    os << std::fixed;
    os.precision(4);
    os << "TransRecord(Datetime(\"" << record.datetime.ymdhms() << "\")" << strip << record.price
       << strip << record.vol << strip << record.direct << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const TransList& data) {
    if (data.size() > 0) {
        os << "TransList{\n  size : " << data.size() << "\n  start: " << data.front().datetime
           << "\n  last : " << data.back().datetime << "\n }";
    } else {
        os << "TransList{\n  size : " << data.size() << "\n }";
    }
    return os;
}

bool HKU_API operator==(const TransRecord& d1, const TransRecord& d2) {
    return (d1.datetime == d2.datetime && (std::fabs(d1.price - d2.price) < 0.0001) &&
            (std::fabs(d1.vol - d2.vol) < 0.0001) && (d1.direct == d2.direct));
}

TransRecord::TransRecord() : datetime(Datetime()), price(0.0), vol(0.0), direct(BUY) {}

TransRecord::TransRecord(const Datetime& datetime, price_t price, price_t vol, DIRECT direct)
: datetime(datetime), price(price), vol(vol), direct(direct) {}

bool TransRecord::isValid() const {
    return datetime == Null<Datetime>() ? false : true;
}

} /* namespace hku */
