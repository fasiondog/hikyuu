/*
 * SelectorBase.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const SelectorBase& st) {
    os << "Selector(" << st.name() << ", " << st.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const SelectorPtr& st) {
    if (st) {
        os << *st;
    } else {
        os << "Selector(NULL)";
    }

    return os;
}

SelectorBase::SelectorBase() : m_name("SelectorBase"), m_count(0), m_pre_date(Datetime::min()) {}

SelectorBase::SelectorBase(const string& name)
: m_name(name), m_count(0), m_pre_date(Datetime::min()) {}

SelectorBase::~SelectorBase() {}

void SelectorBase::clear() {
    m_count = 0;
    m_pre_date = Datetime::min();
    m_sys_list.clear();
}

void SelectorBase::reset() {
    SystemList::const_iterator iter = m_sys_list.begin();
    for (; iter != m_sys_list.end(); ++iter) {
        (*iter)->reset(true, false);
    }

    m_count = 0;
    m_pre_date = Datetime::min();

    _reset();
}

SelectorPtr SelectorBase::clone() {
    SelectorPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = SelectorPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;

    p->m_name = m_name;
    p->m_count = m_count;
    p->m_pre_date = m_pre_date;

    SystemList::const_iterator iter = m_sys_list.begin();
    for (; iter != m_sys_list.end(); ++iter) {
        p->m_sys_list.push_back((*iter)->clone());
    }

    return p;
}

bool SelectorBase::addSystem(const SystemPtr& sys) {
    HKU_ERROR_IF_RETURN(!sys, false, "Try add null sys, will be discard!");
    HKU_ERROR_IF_RETURN(sys->getStock().isNull(), false, "sys has not bind stock!");
    HKU_ERROR_IF_RETURN(!sys->getMM(), false, "sys has not MoneyManager!");
    HKU_ERROR_IF_RETURN(!sys->getSG(), false, "sys has not Siganl!");
    m_sys_list.push_back(sys);
    return true;
}

bool SelectorBase::addStock(const Stock& stock, const SystemPtr& protoSys) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "Try add Null stock, will be discard!");
    HKU_ERROR_IF_RETURN(!protoSys, false, "Try add Null protoSys, will be discard!");
    HKU_ERROR_IF_RETURN(!protoSys->getMM(), false, "protoSys has not MoneyManager!");
    HKU_ERROR_IF_RETURN(!protoSys->getSG(), false, "protoSys has not Siganl!");
    SYSPtr sys = protoSys->clone();
    sys->setStock(stock);
    m_sys_list.push_back(sys);
    return true;
}

bool SelectorBase::addStockList(const StockList& stkList, const SystemPtr& protoSys) {
    HKU_ERROR_IF_RETURN(!protoSys, false, "Try add Null protoSys, will be discard!");
    HKU_ERROR_IF_RETURN(!protoSys->getMM(), false, "protoSys has not MoneyManager!");
    HKU_ERROR_IF_RETURN(!protoSys->getSG(), false, "protoSys has not Siganl!");
    StockList::const_iterator iter = stkList.begin();
    for (; iter != stkList.end(); ++iter) {
        if (iter->isNull()) {
            HKU_WARN("Try add Null stock, will be discard!");
            continue;
        }

        SYSPtr sys = protoSys->clone();
        sys->setStock(*iter);
        m_sys_list.push_back(sys);
    }

    return true;
}

} /* namespace hku */
