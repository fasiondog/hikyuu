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

    /*if (in_sw_list.size() == 0) {
        return;
    }

    price_t total_weight = 0.0;
    auto sw_iter = in_sw_list.begin();
    for (; sw_iter != in_sw_list.end(); ++sw_iter) {
        total_weight += sw_iter->weight;
    }

    if (total_weight == 0.0) {
        return;
    }

    int precision = m_tm->getParam<int>("precision");
    FundsRecord fr = m_tm->getFunds();
    price_t total_funds = fr.cash + fr.market_value;
    price_t per_cash = total_funds / total_weight;
    sw_iter = in_sw_list.begin();
    for (; sw_iter != in_sw_list.end(); ++sw_iter) {
        TMPtr tm = sw_iter->sys->getTM();
        fr = tm->getFunds();
        price_t current_funds = fr.cash + fr.market_value;
        price_t will_funds = per_cash * sw_iter->weight;
        if (current_funds <= will_funds) {
            continue;
        }

        if (fr.market_value <= will_funds) {
            //取出多余的money
        }

        PositionRecordList pos_list = tm->getPositionList();
    }*/
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
    SystemList pure_se_list;
    for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
        if ((*iter)->getTM()->getStockNumber() == 0) {
            pure_se_list.push_back(*iter);
        }
    }

    //获取权重
    SystemWeightList sw_list = _allocateWeight(date, pure_se_list);
    if (sw_list.size() == 0) {
        return;
    }

    //按权重倒序排序
    stable_sort(sw_list.begin(), sw_list.end(),
         boost::bind(std::less<price_t>(),
                     boost::bind(&SystemWeight::weight, _1),
                     boost::bind(&SystemWeight::weight, _2)));
    reverse(sw_list.begin(), sw_list.end());

    size_t remain = max_num - hold_list.size();
    if (remain < sw_list.size()) {
        SystemWeightList temp_list;
        temp_list.insert(temp_list.end(), sw_list.begin(), sw_list.begin() + remain);
        swap(sw_list, temp_list);
    }

    price_t total_weight = 0.0;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        total_weight += sw_iter->weight;
    }

    if (total_weight == 0.0) {
        return;
    }

    int precision = m_tm->getParam<int>("precision");
    price_t per_cash = 0.0;

    per_cash = m_tm->currentCash() / total_weight;
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

        out_sys_list.push_back(sw_iter->sys);
    }
}


SystemWeightList AllocateFundsBase
::allocateWeight(const Datetime& date, const SystemList& se_list) {
    //se_list是当前选中的系统实例、hold_list是当前有持仓的系统实例，两者可能是重复的

    SystemWeightList result;

    int max_num = getParam<int>("max_sys_num");

    result = _allocateWeight(date, se_list);

    stable_sort(result.begin(), result.end(),
                boost::bind(std::less<price_t>(),
                            boost::bind(&SystemWeight::weight, _1),
                            boost::bind(&SystemWeight::weight, _2)));

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
