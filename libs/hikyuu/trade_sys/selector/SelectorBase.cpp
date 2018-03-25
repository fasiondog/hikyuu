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

SelectorBase::SelectorBase()
: m_name("SelectorBase"), m_count(0), m_pre_date(Datetime::min()) {
    setParam<int>("freq", 1); //已Bar为单位
}

SelectorBase::SelectorBase(const string& name)
: m_name(name), m_count(0), m_pre_date(Datetime::min()) {
    setParam<int>("freq", 1);
}


SelectorBase::~SelectorBase() {

}

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
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [SelectorBase::clone]");
        p = SelectorPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [SelectorBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;

    p->m_name = m_name;
    p->m_count = m_count;
    p->m_pre_date = m_pre_date;

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

bool SelectorBase::changed(Datetime date) {
    if (date <= m_pre_date || date == Null<Datetime>())
        return false;

    int freq = getParam<int>("freq");
    if (freq <= 0) {
        freq = 1;
    }

    m_count++;
    if (m_count >= freq){
        m_count = 0;
        m_pre_date = date;
        return true;
    }

    return false;
}


} /* namespace hku */
