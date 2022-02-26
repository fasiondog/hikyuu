/*
 * SelectorBase.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "SelectorBase.h"
#include "../portfolio/Portfolio.h"

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

SelectorBase::SelectorBase() : m_name("SelectorBase") {
    // 是否单独执行原型系统
    setParam<bool>("run_proto_sys", false);
}

SelectorBase::SelectorBase(const string& name) : m_name(name) {
    // 是否单独执行原型系统
    setParam<bool>("run_proto_sys", false);
}

SelectorBase::~SelectorBase() {}

void SelectorBase::clear() {
    m_pro_sys_list.clear();
    m_real_sys_list.clear();
}

void SelectorBase::reset() {
    SystemList::const_iterator iter = m_pro_sys_list.begin();
    for (; iter != m_pro_sys_list.end(); ++iter) {
        // 复位账户但不复位ev
        (*iter)->reset(true, false);
    }

    m_real_sys_list.clear();
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
    p->m_real_sys_list = m_real_sys_list;
    p->m_pro_sys_list = m_pro_sys_list;
    return p;
}

void SelectorBase::calculate(const SystemList& sysList, const KQuery& query) {
    m_real_sys_list = sysList;
    if (getParam<bool>("run_proto_sys")) {
        for (auto& sys : m_pro_sys_list) {
            sys->run(query);
        }
    }
    _calculate();
}

bool SelectorBase::addStock(const Stock& stock, const SystemPtr& protoSys) {
    HKU_ERROR_IF_RETURN(stock.isNull(), false, "Try add Null stock, will be discard!");
    HKU_ERROR_IF_RETURN(!protoSys, false, "Try add Null protoSys, will be discard!");
    HKU_ERROR_IF_RETURN(!protoSys->getMM(), false, "protoSys has not MoneyManager!");
    HKU_ERROR_IF_RETURN(!protoSys->getSG(), false, "protoSys has not Siganl!");
    SYSPtr sys = protoSys->clone();
    sys->reset(true, false);
    sys->setStock(stock);
    m_pro_sys_list.emplace_back(sys);
    return true;
}

bool SelectorBase::addStockList(const StockList& stkList, const SystemPtr& protoSys) {
    HKU_ERROR_IF_RETURN(!protoSys, false, "Try add Null protoSys, will be discard!");
    HKU_ERROR_IF_RETURN(!protoSys->getMM(), false, "protoSys has not MoneyManager!");
    HKU_ERROR_IF_RETURN(!protoSys->getSG(), false, "protoSys has not Siganl!");
    SYSPtr newProtoSys = protoSys->clone();
    // 复位清除之前的数据，避免因原有数据过多导致下面循环时速度过慢
    newProtoSys->reset(true, false);
    StockList::const_iterator iter = stkList.begin();
    for (; iter != stkList.end(); ++iter) {
        if (iter->isNull()) {
            HKU_WARN("Try add Null stock, will be discard!");
            continue;
        }

        SYSPtr sys = newProtoSys->clone();
        sys->setStock(*iter);
        m_pro_sys_list.emplace_back(sys);
    }
    return true;
}

} /* namespace hku */
