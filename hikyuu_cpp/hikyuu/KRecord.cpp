/*
 * KRecord.cpp
 *
 *  Created on: 2013-2-6
 *      Author: fasiondog
 */

#include "KRecord.h"

namespace hku {

const KRecord KRecord::NullKRecord;

HKU_API std::ostream& operator<<(std::ostream& os, const KRecord& record) {
    string strip(", ");
    os << std::fixed;
    (void)os.precision(4);
    os << "KRecord(Datetime(" << record.datetime.ymdhms() << ")" << strip << record.openPrice
       << strip << record.highPrice << strip << record.lowPrice << strip << record.closePrice
       << strip << record.transAmount << strip << record.transCount << ")";
    os.unsetf(std::ostream::floatfield);
    (void)os.precision();
    return os;
}

bool HKU_API operator==(const KRecord& d1, const KRecord& d2) {
    // transAmount/transCount can reach millions, so their near-equality uses a tolerance scaled to
    // magnitude; prices keep the small absolute tolerance
    auto amountClose = [](price_t a, price_t b) {
        return std::fabs(a - b) <= 0.0001 * (1.0 + std::fabs(a) + std::fabs(b));
    };
    return (d1.datetime == d2.datetime && (std::fabs(d1.openPrice - d2.openPrice) < 0.0001) &&
            (std::fabs(d1.highPrice - d2.highPrice) < 0.0001) &&
            (std::fabs(d1.lowPrice - d2.lowPrice) < 0.0001) &&
            (std::fabs(d1.closePrice - d2.closePrice) < 0.0001) &&
            amountClose(d1.transAmount, d2.transAmount) &&
            amountClose(d1.transCount, d2.transCount));
}

bool HKU_API operator!=(const KRecord& d1, const KRecord& d2) {
    return !(d1 == d2);
}

}  // namespace hku
