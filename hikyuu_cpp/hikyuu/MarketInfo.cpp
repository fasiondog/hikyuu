/*
 * MarketInfo.cpp
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketInfo.h"

namespace hku {

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
       << m_openTime2.minutes() << sp << m_closeTime2.hours() << ":" << m_closeTime2.minutes()
       << ")";
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
