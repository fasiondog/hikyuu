/*
 * MarketInfo.cpp
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketInfo.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os,
                                   const MarketInfo& market){
    if(Null<MarketInfo>() == market) {
        os << "MarketInfo()";
        return os;
    }

    string split(", ");
    os << "MarketInfo(" << market.market() << split << market.name() << split
       << market.description() << split << market.code() << split
       << market.lastDate() << ")";
    return os;
}


MarketInfo::MarketInfo() { }

MarketInfo::MarketInfo(const string& market, const string& name,
                       const string& description, const string& code,
                       const Datetime& lastDate)
: m_market(market),
  m_name(name),
  m_description(description),
  m_code(code),
  m_lastDate(lastDate) { }


} /* namespace hikyuu */
