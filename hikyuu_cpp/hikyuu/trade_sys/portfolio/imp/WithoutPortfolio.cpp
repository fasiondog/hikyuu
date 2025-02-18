/*
 * WithoutAFPortfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "WithoutAFPortfolio.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WithoutAFPortfolio)
#endif

namespace hku {

WithoutAFPortfolio::WithoutAFPortfolio() : Portfolio("PF_WithoutAF") {
    initParam();
}

WithoutAFPortfolio::WithoutAFPortfolio(const TradeManagerPtr& tm, const SelectorPtr& se)
: Portfolio("PF_WithoutAF", tm, se, AFPtr()) {
    initParam();
}

WithoutAFPortfolio::~WithoutAFPortfolio() {}

void WithoutAFPortfolio::initParam() {
    setParam<bool>("trade_on_close", true);         // 在收盘时交易
    setParam<bool>("sys_use_self_tm", false);       // 原型系统计算时使用自身附带的tm
    setParam<bool>("sell_at_not_selected", false);  // 强制卖出在选股日未选中的持仓
}

void WithoutAFPortfolio::_reset() {
    m_force_sell_sys_list.clear();
    m_running_sys_list.clear();
    m_selected_list.clear();
    m_se_sys_to_pf_sys_dict.clear();
}

void WithoutAFPortfolio::_readyForRun() {
    // 从 se 获取原型系统列表
    auto pro_sys_list = m_se->getProtoSystemList();
    HKU_WARN_IF_RETURN(pro_sys_list.empty(), void(), "Can't fetch proto_sys_lsit from Selector!");

    // 仅支持都是延迟买卖或都是立刻买卖模式
    bool trade_on_close = getParam<bool>("trade_on_close");
    bool sys_use_self_tm = getParam<bool>("sys_use_self_tm");

    size_t total = pro_sys_list.size();
    m_real_sys_list.reserve(total);
    SystemList se_sys_list;
    se_sys_list.reserve(total);
    for (size_t i = 0; i < total; i++) {
        SystemPtr& pro_sys = pro_sys_list[i];
        if (pro_sys) {
            auto sys = pro_sys->clone();
            sys->setParam<bool>("buy_delay", trade_on_close);
            sys->setParam<bool>("sell_delay", trade_on_close);
            sys->setParam<bool>("shared_tm", true);

            auto se_sys = sys->clone();
            se_sys->setParam<bool>("shared_tm", false);
            if (!sys_use_self_tm || !se_sys->getTM()) {
                // 使用自身 tm 或自身无tm 时，复制使用 pf tm
                se_sys->setTM(m_tm->clone());
                se_sys_list.emplace_back(se_sys);
            }

            m_se->bindRealToProto(se_sys, pro_sys);
            m_real_sys_list.emplace_back(sys);
            m_se_sys_to_pf_sys_dict[se_sys] = sys;

            KData k = sys->getStock().getKData(m_query);
            se_sys->readyForRun();
            se_sys->setTO(k);
            // se_sys->setParam<bool>("trace", true);

            sys->setTM(m_tm);
            string sys_name = fmt::format("{}_{}_{}", sys->name(), sys->getStock().market_code(),
                                          sys->getStock().name());
            sys->name(fmt::format("PF_{}", sys_name));
            sys->readyForRun();
            sys->setTO(k);
        }
    }

    // 告知 se 计算
    m_se->calculate(se_sys_list, m_query);
}

void WithoutAFPortfolio::_runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    if (getParam<bool>("sell_at_not_selected")) {
        _runMomentWithoutAFNotForceSell(date, nextCycle, adjust);
    } else {
        _runMomentWithoutAFForceSell(date, nextCycle, adjust);
    }
}

void WithoutAFPortfolio::_runMomentWithoutAFNotForceSell(const Datetime& date,
                                                         const Datetime& nextCycle, bool adjust) {
    // m_running_sys_set 当前运行系统集合
    // m_selected_list 本轮周期选择的系统列表
    // m_delay_adjust_sys_list 当前周期中未选中但仍在运行中的系统列表，等待无持仓后从运行集合中移除

    // bool trace = getParam<bool>("trace");
    // bool trade_on_close = getParam<bool>("trade_on_close");

    // if (!trade_on_close) {
    //     // 开盘处理
    // }

    // // 调仓日，重新选择系统池
    // if (adjust) {
    // }

    // if (trade_on_close) {
    // }
}

void WithoutAFPortfolio::_runMomentWithoutAFForceSell(const Datetime& date,
                                                      const Datetime& nextCycle, bool adjust) {
    bool trace = getParam<bool>("trace");

    // 开盘时处理强制卖出系统列表，如果以没有持仓，则从列表中删除，否则执行卖出且保留（下一交易日判断是否仍有持仓）
    for (auto iter = m_force_sell_sys_list.begin(); iter != m_force_sell_sys_list.end();) {
        auto& sys = *iter;
        auto stk = sys->getStock();
        auto num = m_tm->getHoldNumber(date, stk);
        if (iszero(num)) {
            iter = m_force_sell_sys_list.erase(iter);
        } else {
            auto _ = sys->sellForceOnOpen(date, num, PART_PORTFOLIO);
            ++iter;
        }
    }

    //---------------------------------------------------
    // 非调仓日，只需依次执行当前运行中的系统
    //---------------------------------------------------
    if (!adjust) {
        // 非调仓日，依次执行运行中所有系统
        for (auto& sys : m_running_sys_list) {
            sys->runMoment(date);
        }
        return;
    }

    //---------------------------------------------------
    // 调仓日，重新选择系统池
    //---------------------------------------------------
    bool trade_on_close = getParam<bool>("trade_on_close");

    if (adjust) {
        auto current_selected_list = m_se->getSelected(date);
        HKU_INFO_IF(trace, "[PF] current seleect system count: {}", current_selected_list.size());

        m_selected_list.clear();
        for (auto& sw : current_selected_list) {
            m_selected_list.push_back(m_se_sys_to_pf_sys_dict[sw.sys]);
        }

        // 从当前运行池中移除不在选中系统池中的系统
        std::unordered_set<SYSPtr> tmp_selected_set;
        for (auto& sys : m_selected_list) {
            tmp_selected_set.insert(sys);
        }

        std::unordered_set<SYSPtr> will_remove_sys_list;
        for (auto& sys : m_running_sys_set) {
            if (tmp_selected_set.find(sys) == tmp_selected_set.end()) {
                will_remove_sys_list.insert(sys);
            }
        }

        for (auto& sys : will_remove_sys_list) {
            HKU_INFO_IF(trace, "[PF] remove system: {}", sys->name());
            m_running_sys_set.erase(sys);
            m_running_sys_list.remove(sys);
        }

        // 移除的系统如果有持仓，则强制立刻卖出，且为开盘时执行
        if (!trade_on_close) {
            for (auto& sys : will_remove_sys_list) {
                auto stk = sys->getStock();
                auto num = m_tm->getHoldNumber(date, stk);
                auto _ = sys->sellForceOnOpen(date, num, PART_PORTFOLIO);
                m_force_sell_sys_list.emplace_back(sys);
            }
        }

        // 加入当前运行系统集合，并设置调仓周期
        for (auto& sys : m_selected_list) {
            m_running_sys_set.insert(sys);
            m_running_sys_list.emplace_back(sys);
            auto sg = sys->getSG();
            sg->startCycle(date, nextCycle);
        }
        //----------------------------------------------------------------------------
        // 依次执行运行中所有系统
        //----------------------------------------------------------------------------
        for (auto& sys : m_running_sys_list) {
            sys->runMoment(date);
        }

        // 如果是收盘时执行模式，则将待移除系统中的持仓卖出
        if (trade_on_close) {
            for (auto& sys : will_remove_sys_list) {
                auto stk = sys->getStock();
                auto num = m_tm->getHoldNumber(date, stk);
                auto _ = sys->sellForceOnClose(date, num, PART_PORTFOLIO);
                m_force_sell_sys_list.emplace_back(sys);
            }
        }
    }
}

PortfolioPtr HKU_API PF_WithoutAF(const TMPtr& tm, const SEPtr& se, int adjust_cycle,
                                  const string& adjust_mode, bool delay_to_trading_day,
                                  bool trade_on_close, bool sys_use_self_tm) {
    PortfolioPtr ret = make_shared<WithoutAFPortfolio>(tm, se);
    ret->setParam<int>("adjust_cycle", adjust_cycle);
    ret->setParam<string>("adjust_mode", adjust_mode);
    ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
    ret->setParam<bool>("trade_on_close", trade_on_close);
    ret->setParam<bool>("sys_use_self_tm", sys_use_self_tm);
    return ret;
}

} /* namespace hku */
