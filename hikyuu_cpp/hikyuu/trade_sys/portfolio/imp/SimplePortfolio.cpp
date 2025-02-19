/*
 * SimplePortfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "hikyuu/global/sysinfo.h"
#include "hikyuu/trade_manage/crt/crtTM.h"
#include "hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h"

#include "SimplePortfolio.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SimplePortfolio)
#endif

namespace hku {

SimplePortfolio::SimplePortfolio() : Portfolio("PF_Simple") {}

SimplePortfolio::SimplePortfolio(const TradeManagerPtr& tm, const SelectorPtr& se, const AFPtr& af)
: Portfolio("PF_Simple", tm, se, af) {}

SimplePortfolio::~SimplePortfolio() {}

void SimplePortfolio::_reset() {
    m_dlist_sys_list.clear();
    m_delay_adjust_sys_list.clear();
    m_tmp_selected_list.clear();
    m_tmp_will_remove_sys.clear();
}

void SimplePortfolio::_readyForRun() {
    HKU_CHECK(m_af, "m_af is null!");

    // se算法和af算法不匹配
    HKU_CHECK(m_se->isMatchAF(m_af), "The current SE and AF do not match!");

    // 检查账户是否存在初始资产
    FundsRecord funds = m_tm->getFunds();
    HKU_CHECK(funds.total_assets() > 0.0, "The current tm is zero assets!");

    // 从 se 获取原型系统列表
    const auto& pro_sys_list = m_se->getProtoSystemList();
    HKU_WARN_IF_RETURN(pro_sys_list.empty(), void(), "Can't fetch proto_sys_lsit from Selector!");

    // 生成资金账户
    m_cash_tm = m_tm->clone();

    // 配置资产分配器
    m_af->setTM(m_tm);
    m_af->setCashTM(m_cash_tm);
    m_af->setQuery(m_query);

    // 获取所有备选子系统，为无关联账户的子系统分配子账号，对所有子系统做好启动准备
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "TM_SUB");
    size_t total = pro_sys_list.size();
    m_real_sys_list.reserve(total);
    for (size_t i = 0; i < total; i++) {
        const SystemPtr& pro_sys = pro_sys_list[i];
        if (pro_sys) {
            SystemPtr sys = pro_sys->clone();
            m_se->bindRealToProto(sys, pro_sys);
            m_real_sys_list.emplace_back(sys);

            // 为内部实际执行的系统创建初始资金为0的子账户
            sys->setTM(pro_tm->clone());
            string sys_name = fmt::format("{}_{}_{}", sys->name(), sys->getStock().market_code(),
                                          sys->getStock().name());
            sys->getTM()->name(fmt::format("TM_SUB_{}", sys_name));
            sys->name(fmt::format("PF_{}", sys_name));

            sys->readyForRun();
            KData k = sys->getStock().getKData(m_query);
            sys->setTO(k);
        }
    }

    // 告知 se 当前实际运行的系统列表
    m_se->calculate(m_real_sys_list, m_query);
}

void SimplePortfolio::_runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    //---------------------------------------------------
    // 检测运行系统中是否存在已退市的证券
    //---------------------------------------------------
    for (auto iter = m_running_sys_set.begin(); iter != m_running_sys_set.end(); /*++iter*/) {
        auto& sys = *iter;
        if (sys->getStock().getMarketValue(date, m_query.kType()) == 0.0) {
            auto sub_tm = sys->getTM();
            auto sub_cash = sub_tm->currentCash();
            if (sub_cash > 0.0 && sub_tm->checkout(date, sub_cash)) {
                m_cash_tm->checkin(date, sub_cash);
            }
            m_dlist_sys_list.emplace_back(sys);
            m_running_sys_set.erase(iter++);
        } else {
            ++iter;
        }
    }

    //---------------------------------------------------
    // 开盘前处理各个子账户、资金账户、总账户之间可能的误差
    //---------------------------------------------------
    int precision = m_tm->getParam<int>("precision");

    // 更新所有运行中系统的权息
    price_t sum_cash = 0.0;
    for (auto& running_sys : m_running_sys_set) {
        TMPtr sub_tm = running_sys->getTM();
        sub_tm->updateWithWeight(date);
        sum_cash += sub_tm->currentCash();
    }

    // 开盘前，进行轧差处理（平衡 sub_sys, cash_tm, tm 之间的误差）
    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[PF] The sum cash of sub_tm: {}, cash tm: {}, tm cash: {}", sum_cash,
                m_cash_tm->currentCash(), m_tm->currentCash());
    sum_cash += m_cash_tm->currentCash();

    price_t diff = roundEx(std::abs(m_tm->currentCash() - sum_cash), precision);
    if (diff > 0.) {
        if (m_tm->currentCash() > sum_cash) {
            m_cash_tm->checkin(date, diff);
        } else if (m_tm->currentCash() < sum_cash) {
            if (m_cash_tm->currentCash() > diff) {
                m_cash_tm->checkout(date, m_cash_tm->currentCash() - diff);
            }
        }
        HKU_INFO_IF(trace, "After compensate: the sum cash of sub_tm: {}, cash tm: {}, tm cash: {}",
                    sum_cash, m_cash_tm->currentCash(), m_tm->currentCash());
    }

    //----------------------------------------------------------------------
    // 跟踪打印执行调仓前的资产情况
    //----------------------------------------------------------------------
    if (trace) {
        auto funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] [beforce adjust] - total funds: {},  cash: {}, market_value: {}",
                 funds.cash + funds.market_value, funds.cash, funds.market_value);
    }

    //----------------------------------------------------------------------
    // 开盘时，优先处理上一交易日遗留的延迟调仓卖出的系统
    //----------------------------------------------------------------------
    HKU_INFO_IF(trace, "[PF] process delay adjust sys, size: {}", m_delay_adjust_sys_list.size());
    SystemWeightList tmp_continue_adjust_sys_list;
    for (auto& sys : m_delay_adjust_sys_list) {
        auto tr = sys.sys->sellForceOnOpen(date, sys.weight, PART_PORTFOLIO);
        if (!tr.isNull()) {
            HKU_INFO_IF(trace, "[PF] Delay adjust sell: {}", tr);
            m_tm->addTradeRecord(tr);

            // 卖出后，尝试将资金取出转移至影子总账户
            TMPtr sub_tm = sys.sys->getTM();
            auto sub_cash = sub_tm->currentCash();
            if (sub_cash > 0.0 && sub_tm->checkout(date, sub_cash)) {
                m_cash_tm->checkin(date, sub_cash);
            }

        } else {
            // 强制卖出失败的情况下，如果当前仍有持仓，则需要下一交易日继续进行处理
            PositionRecord position = sys.sys->getTM()->getPosition(date, sys.sys->getStock());
            if (position.number > 0.0) {
                HKU_INFO_IF(trace, "[{}] failed to force sell, delay to next day", name());
                tmp_continue_adjust_sys_list.emplace_back(sys);
            }
        }
    }

    m_delay_adjust_sys_list.swap(tmp_continue_adjust_sys_list);

    //---------------------------------------------------
    // 检测当前运行中的系统是否存在延迟卖出信号，并在开盘时有效处理
    //---------------------------------------------------
    for (auto& sys : m_running_sys_set) {
        auto tr = sys->pfProcessDelaySellRequest(date);
        if (!tr.isNull()) {
            HKU_INFO_IF(trace, "[PF] sell delay {}", tr);
            m_tm->addTradeRecord(tr);
        }
    }

    //---------------------------------------------------
    // 调仓日，进行资金分配调整
    //---------------------------------------------------
    if (adjust) {
        // 从选股策略获取选中的系统列表
        m_tmp_selected_list = m_se->getSelected(date);

        // 如果选中的系统不在已有列表中, 则先清除其延迟买入操作，防止在调仓日出现未来信号
        for (auto& sys : m_tmp_selected_list) {
            if (sys.sys) {
                if (m_running_sys_set.find(sys.sys) == m_running_sys_set.end()) {
                    sys.sys->clearDelayBuyRequest();
                }
            }
        }

        if (trace && !m_tmp_selected_list.empty()) {
            for (auto& sys : m_tmp_selected_list) {
                HKU_INFO_IF(sys.sys, "[PF] select: {}, score: {:<.4f}", sys.sys->name(),
                            sys.weight);
            }
        }

        // 资产分配算法调整各子系统资产分配
        tmp_continue_adjust_sys_list =
          m_af->adjustFunds(date, m_tmp_selected_list, m_running_sys_set);

        if (m_delay_adjust_sys_list.empty()) {
            m_delay_adjust_sys_list.swap(tmp_continue_adjust_sys_list);
        } else {
            for (auto& sw : tmp_continue_adjust_sys_list) {
                m_delay_adjust_sys_list.emplace_back(sw);
            }
        }

        // 如果选中的系统不在已有列表中，且账户已经被分配了资金，则将其加入运行系统列表
        for (auto& sys : m_tmp_selected_list) {
            if (sys.sys) {
                if (m_running_sys_set.find(sys.sys) == m_running_sys_set.end()) {
                    if (sys.sys->getTM()->cash(date, m_query.kType()) > 0.0) {
                        m_running_sys_set.insert(sys.sys);
                    }
                }
            }
        }

        // 从已运行系统列表中立即移除已没有持仓且没有资金的系统
        m_tmp_will_remove_sys.clear();
        for (auto& sys : m_running_sys_set) {
            auto sub_tm = sys->getTM();
            if (sub_tm->currentCash() < 1.0 && 0 == sub_tm->getHoldNumber(date, sys->getStock())) {
                m_tmp_will_remove_sys.emplace_back(sys, 0.0);
            }
        }

        for (auto& sw : m_tmp_will_remove_sys) {
            m_running_sys_set.erase(sw.sys);
        }
    }

    //----------------------------------------------------------------------
    // 跟踪打印执行调仓后的资产情况
    //----------------------------------------------------------------------
    if (trace) {
        auto funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] [after adjust] - total funds: {},  cash: {}, market_value: {}",
                 funds.total_assets(), funds.cash, funds.market_value);
    }

    //----------------------------------------------------------------------------
    // 执行所有运行中的系统，无论是延迟还是非延迟，当天运行中的系统都需要被执行一次
    //----------------------------------------------------------------------------
    for (auto& sub_sys : m_running_sys_set) {
        HKU_INFO_IF(trace, "[PF] run: {}", sub_sys->name());
        if (adjust) {
            auto sg = sub_sys->getSG();
            sg->startCycle(date, nextCycle);
            HKU_INFO_IF(trace, "[PF] sg should buy: {}", sg->shouldBuy(date));
        }

        auto tr = sub_sys->runMoment(date);
        if (!tr.isNull()) {
            HKU_INFO_IF(trace, "[PF] {}", tr);
            m_tm->addTradeRecord(tr);
        }
    }

    //----------------------------------------------------------------------
    // 跟踪各个子系统执行后的资产情况
    //----------------------------------------------------------------------
    if (trace) {
        auto funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] [after run] - total funds: {},  cash: {}, market_value: {}",
                 funds.cash + funds.market_value, funds.cash, funds.market_value);
    }
}

PortfolioPtr HKU_API PF_Simple(const TMPtr& tm, const SEPtr& st, const AFPtr& af, int adjust_cycle,
                               const string& adjust_mode, bool delay_to_trading_day) {
    PortfolioPtr ret = make_shared<SimplePortfolio>(tm, st, af);
    ret->setParam<int>("adjust_cycle", adjust_cycle);
    ret->setParam<string>("adjust_mode", adjust_mode);
    ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
    return ret;
}

} /* namespace hku */
