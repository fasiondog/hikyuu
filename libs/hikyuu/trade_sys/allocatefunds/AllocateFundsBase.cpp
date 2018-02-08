/*
 * AllocateMoney.cpp
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#include <boost/bind.hpp>
#include "AllocateFundsBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const AllocateFundsBase& af) {
    os << "AllocateFunds(" << af.name() << ", " << af.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const AFPtr& af) {
    if (af) {
        os << *af;
    } else {
        os << "AllocateFunds(NULL)";
    }

    return os;
}

AllocateFundsBase::AllocateFundsBase(): m_name("AllocateMoneyBase") {
    setParam<bool>("adjust_hold_sys", false); //是否调整之前已经持仓策略的持仓
    setParam<int>("max_sys_num", 10); //最大系统实例数
}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase") {
    setParam<bool>("adjust_hold_sys", false);
    setParam<int>("max_sys_num", 10); //最大系统实例数
}

AllocateFundsBase::~AllocateFundsBase() {

}

AFPtr AllocateFundsBase::clone() {
    AFPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [AllocateMoneyBase::clone]");
        p = AFPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [AllocateMoneyBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_tm = m_tm;
    return p;
}

SystemList AllocateFundsBase
::getAllocateSystem(const Datetime& date,
        const SystemList& se_list, const SystemList& hold_list) {
    SystemList result;

    SystemWeightList sw_list = allocateWeight(se_list, hold_list);

    /*sort(sw_list.begin(), sw_list.end(),
         boost::bind(std::less<price_t>(),
                     boost::bind(&SystemWeight::weight, _1),
                     boost::bind(&SystemWeight::weight, _2)));
    */
    price_t total_weight = 0.0;

    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        total_weight += sw_iter->weight;
    }

    if (total_weight == 0.0) {
        return result;
    }

    int precision = m_tm->getParam<int>("precision");
    price_t per_cash = roundDown(m_tm->currentCash() / total_weight, precision);

    sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t will_cash = roundDown(per_cash * sw_iter->weight, precision);
        if (will_cash == 0.0) {
            continue;
        }

        TMPtr tm = sw_iter->sys->getTM();
        precision = tm->getParam<int>("precision");
        will_cash = roundDown(will_cash, precision);
        if (tm->currentCash() < will_cash) {
            price_t cash = will_cash - tm->currentCash();
            tm->checkin(date, cash);
            m_tm->checkout(date, cash);
        }

        result.push_back(sw_iter->sys);
    }

    return result;
}

SystemWeightList AllocateFundsBase
::allocateWeight(const SystemList& se_list, const SystemList& hold_list) {
    SystemWeightList result;

    result = _allocateWeight(se_list, hold_list);

    size_t total = result.size();
    if (total <= getParam<int>("max_sys_num")) {
        return result;
    }

    auto iter = result.begin() + total;
    SystemWeightList sw_list;
    copy(result.begin(), iter, sw_list.begin());
    swap(result, sw_list);

    return result;
}


} /* namespace hku */
