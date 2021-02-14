/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-13
 *     Author: fasiondog
 */

#include "crt/TC_Zero.h"
#include "TradeManagerBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const TradeManagerBase& tm) {
    os << tm.str();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const TradeManagerPtr& ptm) {
    if (ptm) {
        os << ptm->str();
    } else {
        os << "TradeManager(NULL)";
    }
    return os;
}

TradeManagerBase::TradeManagerBase()
: m_broker_last_datetime(Datetime::now()), m_costfunc(TC_Zero()) {
    setParam<int>("precision", 2);  //计算精度
}

shared_ptr<TradeManagerBase> TradeManagerBase::clone() {
    shared_ptr<TradeManagerBase> p = _clone();
    HKU_CHECK(p, "Invalid ptr from _clone!");
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_broker_last_datetime = m_broker_last_datetime;
    p->m_costfunc = m_costfunc;
    return p;
}

}  // namespace hku