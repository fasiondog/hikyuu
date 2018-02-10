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
::getAllocatedSystemList(const Datetime& date,
        const SystemList& se_list, const SystemList& hold_list) {
    SystemList result;

    SystemWeightList sw_list = allocateWeight(se_list, hold_list);
    if (sw_list.size() == 0) {
        return result;
    }

    price_t total_weight = 0.0;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        total_weight += sw_iter->weight;
    }

    if (total_weight == 0.0) {
        return result;
    }

    int precision = m_tm->getParam<int>("precision");
    price_t per_cash = m_tm->currentCash() / total_weight;
    sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t will_cash = per_cash * sw_iter->weight;
        if (will_cash == 0.0) {
            continue;
        }

        TMPtr tm = sw_iter->sys->getTM();
        assert(tm);

        int real_precision = tm->getParam<int>("precision");
        if (precision < real_precision) {
            real_precision = precision;
        }

        will_cash = roundDown(will_cash - tm->currentCash(), real_precision);
        if (will_cash > 0) {
            if (!m_tm->checkout(date, will_cash)) {
                HKU_ERROR("m_tm->checkout failed! "
                        "[AllocateFundsBase::getAllocatedSystemList]");
                continue;
            }
            tm->checkin(date, will_cash);
        }

        result.push_back(sw_iter->sys);
    }

    return result;
}

SystemWeightList AllocateFundsBase
::allocateWeight(const SystemList& se_list, const SystemList& hold_list) {
    SystemWeightList result;

    int max_num = getParam<int>("max_sys_num");
    if (max_num <= 0) {
        HKU_ERROR("param(max_sys_num) need > 0! [AllocateFundsBase::allocateWeight]");
        return result;
    }

    if (getParam<bool>("adjust_hold_sys")) {
        if (se_list.size() == 0 && hold_list.size() == 0) {
            return result;
        }

        result = _allocateWeight(se_list, hold_list);

    } else {
        if (se_list.size() == 0) {
            return result;
        }

        result = _allocateWeight(se_list, SystemList());
    }

    size_t total = result.size();
    if (total <= max_num) {
        return result;
    }

    auto iter = result.begin() + max_num;
    SystemWeightList sw_list;
    sw_list.insert(sw_list.end(), result.begin(), iter);
    //swap(result, sw_list);

    return sw_list;
}

} /* namespace hku */
