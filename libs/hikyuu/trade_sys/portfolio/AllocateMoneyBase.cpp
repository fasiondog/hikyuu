/*
 * AllocateMoneyBase.cpp
 *
 *  Created on: 2016年3月23日
 *      Author: Administrator
 */

#include <hikyuu/trade_sys/portfolio/AllocateMoneyBase.h>

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const AllocateMoneyBase& am) {
    os << "AllocateMoneyBase(" << am.name() << ", " << am.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const AllocateMoneyPtr& am) {
    if (am) {
        os << *am;
    } else {
        os << "AllocateMoneyBase(NULL)";
    }

    return os;
}


AllocateMoneyBase::AllocateMoneyBase() {

}

AllocateMoneyBase::AllocateMoneyBase(const TradeManagerPtr& tm): m_tm(tm) {

}

AllocateMoneyBase::~AllocateMoneyBase() {

}

void AllocateMoneyBase::reset() {
    _reset();
}

AllocateMoneyPtr AllocateMoneyBase::clone() {
    AllocateMoneyPtr p = _clone();
    if (p) {
        p->m_params = m_params;
        p->m_tm = m_tm;
    }
    return p;
}


StockList AllocateMoneyBase::tryAllocate(Datetime date, const StockList& stocks) {
    return stocks;
}


} /* namespace hku */
