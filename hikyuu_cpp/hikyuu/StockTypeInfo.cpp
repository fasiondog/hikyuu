/*
 * StockTypeInfo.cpp
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#include "StockTypeInfo.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const StockTypeInfo& stockTypeInfo) {
    if (Null<StockTypeInfo>() == stockTypeInfo) {
        os << "StockTypeInfo()";
        return os;
    }

    string sp(", ");
    os << "StockTypeInfo(" << stockTypeInfo.type() << sp << stockTypeInfo.description() << sp
       << stockTypeInfo.tick() << sp << stockTypeInfo.tickValue() << sp << stockTypeInfo.unit()
       << sp << stockTypeInfo.precision() << sp << stockTypeInfo.minTradeNumber() << sp
       << stockTypeInfo.maxTradeNumber() << ")";
    return os;
}

string StockTypeInfo::toString() const {
    std::stringstream os;
    if (Null<StockTypeInfo>() == *this) {
        os << "StockTypeInfo()";
        return os.str();
    }

    string sp(", ");
    os << "StockTypeInfo(" << m_type << sp << m_description << sp << m_tick << sp << m_tickValue
       << sp << m_unit << sp << m_precision << sp << m_minTradeNumber << sp << m_maxTradeNumber
       << ")";
    return os.str();
}

StockTypeInfo::StockTypeInfo()
: m_type(Null<uint32_t>()),
  m_tick(0.0),
  m_tickValue(0.0),
  m_unit(1.0),
  m_precision(0),
  m_minTradeNumber(0),
  m_maxTradeNumber(0) {}

StockTypeInfo::StockTypeInfo(uint32_t type, const string& description, price_t tick,
                             price_t tickValue, int precision, double minTradeNumber,
                             double maxTradeNumber)
: m_type(type),
  m_description(description),
  m_tick(tick),
  m_tickValue(tickValue),
  m_precision(precision),
  m_minTradeNumber(minTradeNumber),
  m_maxTradeNumber(maxTradeNumber) {
    if (m_tick == 0.0) {
        m_unit = 1.0;
        HKU_WARN("tick should not be zero!");
    } else {
        m_unit = m_tickValue / m_tick;
    }
}

}  // namespace hku
