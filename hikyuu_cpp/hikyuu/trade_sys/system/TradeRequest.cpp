/*
 * TradeRequest.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "TradeRequest.h"
#include "SystemPart.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const TradeRequest& tr) {
    os << "TradeRequest(" << tr.valid << ", " << getBusinessName(tr.business) << ", " << tr.datetime
       << ", " << std::fixed << std::setprecision(4) << tr.stoploss << ", " << std::fixed
       << std::setprecision(4) << tr.goal << ", " << std::fixed << std::setprecision(4) << tr.number
       << ", " << getSystemPartName(tr.from) << ", " << tr.count << ")";
    return os;
}

void TradeRequest::clear() noexcept {
    valid = false;
    business = BUSINESS_INVALID;
    datetime = Null<Datetime>();
    stoploss = 0.0;
    goal = 0.0;
    number = 0.0;
    from = PART_INVALID;
    count = 0;
}

} /* namespace hku */
