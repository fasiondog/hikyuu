/*
 * SelectorBase.cpp
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const SelectorBase& st) {
    os << st.str();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const SelectorPtr& st) {
    if (st) {
        os << st->str();
    } else {
        os << "Selector(NULL)";
    }

    return os;
}

string SelectorBase::str() const {
    std::ostringstream buf;
    buf << "Selector(" << name() << ", " << getParameter() << ", " << m_sc_filter << ")";
    return buf.str();
}

SelectorBase::SelectorBase() : m_name("SelectorBase") {
    initParam();
}

SelectorBase::SelectorBase(const string& name) : m_name(name) {
    initParam();
}

SelectorBase::~SelectorBase() {}

void SelectorBase::initParam() {
    // Usually the prototype system does not participate in the calculation, but in some special
    // scenarios it needs to rely on the companion system strategy; in that case the behavior of the
    // actually executed system can be considered to follow the buys and sells of the companion
    // system, such as a selection relying on the SG (however relying on the SG only is not
    // rigorous, because the SG of the prototype and of the actual system are the same). In this
    // case the prototype
    setParam<bool>("depend_on_proto_sys", false);  // The prototype system must be able to run alone
    setParam<int>("get_n", 0);                     // How many leading items getSelected returns
}

void SelectorBase::baseCheckParam(const string& name) const {}

void SelectorBase::paramChanged() {
    m_calculated = false;
    m_proto_calculated = false;
}

void SelectorBase::removeAll() {
    m_pro_sys_list.clear();
    _removeAll();
    reset();
}

void SelectorBase::reset() {
    SystemList::const_iterator iter = m_pro_sys_list.begin();
    for (; iter != m_pro_sys_list.end(); ++iter) {
        (*iter)->reset();
    }

    m_real_sys_list.clear();
    _reset();

    m_calculated = false;
    m_proto_calculated = false;
}

SelectorPtr SelectorBase::clone() {
    SelectorPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_query = m_query;
    p->m_proto_query = m_proto_query;
    p->m_calculated = m_calculated;
    p->m_proto_calculated = m_proto_calculated;

    p->m_real_sys_list.reserve(m_real_sys_list.size());
    for (const auto& sys : m_real_sys_list) {
        p->m_real_sys_list.emplace_back(sys->clone());
    }

    p->m_pro_sys_list.reserve(m_pro_sys_list.size());
    for (const auto& sys : m_pro_sys_list) {
        p->m_pro_sys_list.emplace_back(sys->clone());
    }

    if (m_sc_filter) {
        p->m_sc_filter = m_sc_filter->clone();
    }

    p->m_pf = m_pf;  // A reference to PF only, not cloned

    return p;
}

void SelectorBase::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    HKU_IF_RETURN(m_calculated && m_query == query, void());

    m_query = query;
    m_real_sys_list = pf_realSysList;

    // It depends on the running system and must be calculated before its own calculation
    if (getParam<bool>("depend_on_proto_sys")) {
        calculate_proto(query);
    }

    _calculate();
    m_calculated = true;
}

void SelectorBase::calculate_proto(const KQuery& query) {
    if (m_proto_query != query && !m_proto_calculated) {
        HKU_WARN_IF_RETURN(m_pro_sys_list.empty(), void(), "m_pro_sys_list is empty!");
        for (auto& sys : m_pro_sys_list) {
            sys->run(query);
        }
        m_proto_calculated = true;
        m_proto_query = query;
    }
}

void SelectorBase::addSystem(const SYSPtr& sys) {
    HKU_CHECK(sys, "The input sys is null!");
    HKU_CHECK(sys->getMM(), "protoSys missing MoneyManager!");
    HKU_CHECK(sys->getSG(), "protoSys missing Siganl!");
    HKU_CHECK(!sys->getParam<bool>("shared_tm"), "Unsupport shared TM for sys!");
    if (getParam<bool>("depend_on_proto_sys")) {
        HKU_CHECK(sys->getTM(), "Scenarios that depend on prototype systems need to specify a TM!");
    }

    sys->reset();
    _addSystem(sys);

    m_pro_sys_list.emplace_back(sys);
    m_calculated = false;
    m_proto_calculated = false;
}

void SelectorBase::addSystemList(const SystemList& sysList) {
    for (const auto& sys : sysList) {
        addSystem(sys);
    }
}

void SelectorBase::addStock(const Stock& stock, const SystemPtr& protoSys) {
    HKU_CHECK(!stock.isNull(), "The input stock is null!");
    HKU_CHECK(protoSys, "The input protoSys is null!");
    HKU_CHECK(protoSys->getMM(), "protoSys missing MoneyManager!");
    HKU_CHECK(protoSys->getSG(), "protoSys missing Siganl!");
    HKU_CHECK(!protoSys->getParam<bool>("shared_tm"), "Unsupport shared TM for protoSys!");
    if (getParam<bool>("depend_on_proto_sys")) {
        HKU_CHECK(protoSys->getTM(),
                  "Scenarios that depend on prototype systems need to specify a TM!");
    }

    auto proto = protoSys;
    proto->forceResetAll();
    SYSPtr sys = proto->clone();
    sys->reset();
    sys->setStock(stock);
    _addSystem(sys);
    m_pro_sys_list.emplace_back(sys);

    m_calculated = false;
    m_proto_calculated = false;
}

void SelectorBase::addStockList(const StockList& stkList, const SystemPtr& protoSys) {
    for (const auto& stk : stkList) {
        addStock(stk, protoSys);
    }
}

SystemWeightList SelectorBase::getSelected(Datetime date) {
    if (getParam<int>("get_n") <= 0) {
        return _getSelected(date);
    }

    SystemWeightList ret = _getSelected(date);
    if (ret.size() > getParam<int>("get_n")) {
        ret.resize(getParam<int>("get_n"));
    }
    return ret;
}

} /* namespace hku */
