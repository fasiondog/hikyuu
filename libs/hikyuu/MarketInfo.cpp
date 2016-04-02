/*
 * MarketInfo.cpp
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketInfo.h"
#include "utilities/util.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os,
                                   const MarketInfo& market){
    if(Null<MarketInfo>() == market) {
        os << "MarketInfo()";
        return os;
    }

    string split(", ");
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX >= 0x03000000)
    os << "MarketInfo(" << market.market() << split
       << utf8_to_gb(market.name()) << split
       << utf8_to_gb(market.description()) << split << market.code() << split
       << market.lastDate() << ")";

#else
    os << "MarketInfo(" << market.market() << split << market.name() << split
       << market.description() << split << market.code() << split
       << market.lastDate() << ")";
#endif
    return os;
}

string MarketInfo::toString() const {
    std::stringstream os;
    if (m_market == "") {
        os << "MarketInfo()";
        return os.str();
    }

    string split(", ");
    os << "MarketInfo(" << m_market << split << m_name << split
       << m_description << split << m_code << split
       << m_lastDate << ")";
    return os.str();
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
