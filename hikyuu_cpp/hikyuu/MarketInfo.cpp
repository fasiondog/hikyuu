/*
 * MarketInfo.cpp
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketInfo.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const MarketInfo& market) {
    if (Null<MarketInfo>() == market) {
        os << "MarketInfo()";
        return os;
    }

    string split(", ");
    os << "MarketInfo(" << market.market() << split << market.name() << split
       << market.description() << split << market.code() << split << market.lastDate() << ")";
    return os;
}

string MarketInfo::toString() const {
    std::stringstream os;
    if (m_market == "") {
        os << "MarketInfo()";
        return os.str();
    }

    string split(", ");
    os << "MarketInfo(" << m_market << split << m_name << split << m_description << split << m_code
       << split << m_lastDate << split << m_openTime1.minutes() << split << m_closeTime1.minutes()
       << split << m_openTime2.minutes() << split << m_closeTime2.minutes() << ")";
    return os.str();
}

MarketInfo::MarketInfo() {}

MarketInfo::MarketInfo(const string& market, const string& name, const string& description,
                       const string& code, const Datetime& lastDate, TimeDelta openTime1,
                       TimeDelta closeTime1, TimeDelta openTime2, TimeDelta closeTime2)
: m_market(market),
  m_name(name),
  m_description(description),
  m_code(code),
  m_lastDate(lastDate),
  m_openTime1(openTime1),
  m_closeTime1(closeTime1),
  m_openTime2(openTime2),
  m_closeTime2(closeTime2) {}

}  // namespace hku
