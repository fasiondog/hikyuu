/*
 * MoneyManagerBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "MoneyManagerBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const MoneyManagerBase& mm) {
    os << "MoneyManager(" << mm.name() << ", " << mm.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const MoneyManagerPtr& mm) {
    if (mm) {
        os << *mm;
    } else {
        os << "MoneyManager(NULL)";
    }

    return os;
}

MoneyManagerBase::MoneyManagerBase(const string& name)
: m_name(name), m_ktype(KQuery::DAY) {

}

MoneyManagerBase::~MoneyManagerBase() {

}

void MoneyManagerBase::buyNotify(const TradeRecord&) {

}

void MoneyManagerBase::sellNotify(const TradeRecord&) {

}

MoneyManagerPtr MoneyManagerBase::clone() {
    MoneyManagerPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_tm = m_tm;
    p->m_ktype = m_ktype;
    return p;
}


size_t MoneyManagerBase::getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
    return Null<size_t>();
}

} /* namespace hku */
