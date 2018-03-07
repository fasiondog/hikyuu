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

    int max_num = getParam<int>("max_sys_num");
    if (max_num <= 0) {
        HKU_ERROR("param(max_sys_num) need > 0! "
                  "[AllocateFundsBase::getAllocatedSystemList]");
        return result;
    }

    //SystemWeightList sw_list = allocateWeight(date, se_list, hold_list);

    if (getParam<bool>("adjust_hold_sys")) {
        _getAllocatedSystemList_adjust_hold(date, se_list, hold_list, result);
    } else {
        _getAllocatedSystemList_not_adjust_hold(date, se_list, hold_list, result);
    }

    return result;
}

void AllocateFundsBase::_getAllocatedSystemList_adjust_hold(
        const Datetime& date,
        const SystemList& se_list,
        const SystemList& hold_list,
        SystemList& out_sys_list) {

    //计算当前选中系统列表的权重


    //将未在当前选中的系统列表中的持仓系统，自动清仓卖出
    //将当前选中系统列表中权重为0的已持仓系统，自动清仓卖出
    //如果持仓系统的总资产大于所分配权重的资产，则将其当前亏损最多的卖出到资产平衡位置
    //按分配的权重调整资产现金

    //TODO 待实现
}

void AllocateFundsBase::_getAllocatedSystemList_not_adjust_hold(
        const Datetime& date,
        const SystemList& se_list,
        const SystemList& hold_list,
        SystemList& out_sys_list) {

    if (se_list.size() == 0) {
        return;
    }

    //不调整持仓，先将所有持仓系统加入到输出系统列表中
    out_sys_list.insert(out_sys_list.end(), hold_list.begin(), hold_list.end());

    //如果持仓的系统数已大于等于最大持仓系统数，直接输出已持仓系统列表，并返回
    int max_num = getParam<int>("max_sys_num");
    if (hold_list.size() >= max_num) {
        return;
    }

    //从当前选中的系统列表中将持仓的系统排除
    std::set<SYSPtr> hold_sets;
    for (auto iter = hold_list.begin(); iter != hold_list.end(); ++iter) {
        hold_sets.insert(*iter);
    }

    SystemList pure_se_list;
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        if (hold_sets.find(*iter) == hold_sets.end()) {
            pure_se_list.push_back(*iter);
        }
    }

    //获取权重
    SystemWeightList sw_list = _allocateWeight(date, pure_se_list);
    if (sw_list.size() == 0) {
        return;
    }

    //按权重排序（注意：无法保证等权重的相对顺序，即使用stable_sort也一样，后面要倒序遍历）
    std::sort(sw_list.begin(), sw_list.end(),
         boost::bind(std::less<price_t>(),
                     boost::bind(&SystemWeight::weight, _1),
                     boost::bind(&SystemWeight::weight, _2)));

    //倒序遍历，计算总权重，并在遇到权重为0或等于最大持仓时
    size_t remain = max_num - hold_list.size();
    price_t total_weight = 0.0;
    auto sw_iter = sw_list.rbegin();
    for (size_t count = 0; sw_iter != sw_list.rend(); ++sw_iter, count++) {
        if (sw_iter->weight <= 0.0 || count >= remain)
            break;
        total_weight += sw_iter->weight;
    }

    if (total_weight == 0.0) {
        return;
    }

    auto end_iter = sw_iter;

    int precision = m_tm->getParam<int>("precision");
    price_t per_cash = 0.0;

    per_cash = m_tm->currentCash() / total_weight;
    sw_iter = sw_list.rbegin();
    for (; sw_iter != end_iter; ++sw_iter) {
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

        out_sys_list.push_back(sw_iter->sys);
    }
}


} /* namespace hku */
