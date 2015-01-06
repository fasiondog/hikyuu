/*
 * StockTypeInfo.cpp
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#include "StockTypeInfo.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os,
                                  const StockTypeInfo& stockTypeInfo){
    if(Null<StockTypeInfo>() == stockTypeInfo) {
        os << "StockTypeInfo()";
        return os;
    }

    string split(", ");
    os << "StockTypeInfo(" << stockTypeInfo.type() << split
       << stockTypeInfo.description() << split
       << stockTypeInfo.tick() << split
       << stockTypeInfo.tickValue() << split
       << stockTypeInfo.unit() << split
       << stockTypeInfo.precision() << split
       << stockTypeInfo.minTradeNumber() << split
       << stockTypeInfo.maxTradeNumber() << ")";
    return os;
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
