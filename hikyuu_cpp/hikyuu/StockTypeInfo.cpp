/*
 * StockTypeInfo.cpp
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#include "StockTypeInfo.h"
#include "utilities/util.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os,
                                  const StockTypeInfo& stockTypeInfo){
    if(Null<StockTypeInfo>() == stockTypeInfo) {
        os << "StockTypeInfo()";
        return os;
    }

    string split(", ");
    os << "StockTypeInfo(" << stockTypeInfo.type() << split
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX >= 0x03000000)
       << utf8_to_gb(stockTypeInfo.description()) << split
#else
       << stockTypeInfo.description() << split
#endif
       << stockTypeInfo.tick() << split
       << stockTypeInfo.tickValue() << split
       << stockTypeInfo.unit() << split
       << stockTypeInfo.precision() << split
       << stockTypeInfo.minTradeNumber() << split
       << stockTypeInfo.maxTradeNumber() << ")";
    return os;
}

string StockTypeInfo::toString() const {
    std::stringstream os;
    if(Null<StockTypeInfo>() == *this) {
        os << "StockTypeInfo()";
        return os.str();
    }

    string split(", ");
    os << "StockTypeInfo(" << m_type << split
       << m_description << split
       << m_tick << split
       << m_tickValue << split
       << m_unit << split
       << m_precision << split
       << m_minTradeNumber << split
       << m_maxTradeNumber << ")";
    return os.str();
}

StockTypeInfo::StockTypeInfo()
: m_type(Null<hku_uint32>()),
  m_tick(0.0),
  m_tickValue(0.0),
  m_unit(1.0),
  m_precision(0),
  m_minTradeNumber(0),
  m_maxTradeNumber(0)  { }


StockTypeInfo::StockTypeInfo(hku_uint32 type,
		const string& description, price_t tick,
		price_t tickValue, int precision,
        size_t minTradeNumber, size_t maxTradeNumber)
: m_type(type),
  m_description(description),
  m_tick(tick),
  m_tickValue(tickValue),
  m_precision(precision),
  m_minTradeNumber(minTradeNumber),
  m_maxTradeNumber(maxTradeNumber)  {
    if (m_tick == 0.0) {
        m_unit = 1.0;
        HKU_WARN("tick should not be zero! [StockTypeInfo::StockTypeInfo]");
    } else {
        m_unit = m_tickValue / m_tick;
    }
}


} /* namespace hikyuu */
