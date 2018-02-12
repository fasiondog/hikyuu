/*
 * SelectorBase.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const SelectorBase& st) {
    os << "Selector(" << st.name() << ", " << st.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const SelectorPtr& st) {
    if (st) {
        os << *st;
    } else {
        os << "Selector(NULL)";
    }

    return os;
}

SelectorBase::SelectorBase(): m_name("SelectorBase") {

}

SelectorBase::SelectorBase(const string& name): m_name(name) {

}


SelectorBase::~SelectorBase() {

}

void SelectorBase::clear() {
    m_sys_list.clear();
}

void SelectorBase::reset() {
    SystemList::const_iterator iter = m_sys_list.begin();
    for (; iter != m_sys_list.end(); ++iter) {
        (*iter)->reset(true, false);
    }
    _reset();
}

SelectorPtr SelectorBase::clone() {
    SelectorPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [SelectorBase::clone]");
        p = SelectorPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [SelectorBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;

    SystemList::const_iterator iter = m_sys_list.begin();
    for (; iter != m_sys_list.end(); ++iter) {
        p->m_sys_list.push_back((*iter)->clone(true, false));
    }

    return p;
}


void SelectorBase::addStock(const Stock& stock, const SystemPtr& protoSys) {
    if (stock.isNull()) {
        HKU_WARN("Try add Null stock, will be discard! "
                "[SelectorBase::addStock]");
        return;
    }

    if (!protoSys) {
        HKU_WARN("Try add Null protoSys, will be discard! "
                "[SelectorBase::addStock]");
        return;
    }

    SYSPtr sys = protoSys->clone(true, false);
    sys->setStock(stock);
    m_sys_list.push_back(sys);
}


void SelectorBase::addStockList(const StockList& stkList,
                                const SystemPtr& protoSys) {
    if (!protoSys) {
        HKU_WARN("Try add Null protoSys, will be discard! "
                "[SelectorBase::addStockList]");
        return;
    }

    StockList::const_iterator iter = stkList.begin();
    for (; iter != stkList.end(); ++iter) {
        if (iter->isNull()) {
            HKU_WARN("Try add Null stock, will be discard! "
                    "[SelectorBase::addStockList]");
            continue;
        }

        SYSPtr sys = protoSys->clone(true, false);
        m_sys_list.push_back(sys);
    }
}


} /* namespace hku */
