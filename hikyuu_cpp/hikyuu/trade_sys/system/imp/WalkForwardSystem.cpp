/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/selector/crt/SE_Optimal.h"
#include "hikyuu/trade_sys/selector/imp/OptimalSelector.h"
#include "WalkForwardSystem.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WalkForwardSystem)
#endif

namespace hku {

WalkForwardSystem::WalkForwardSystem() : System("SYS_Optimal"), m_se(SE_Optimal()) {}

WalkForwardSystem::WalkForwardSystem(const SystemList& candidate_sys_list)
: System("SYS_Optimal"), m_se(SE_Optimal()), m_candidate_sys_list(candidate_sys_list) {}

void WalkForwardSystem::_reset() {
    m_kdata_list.clear();
    m_cur_kdata = 0;
    m_cur_sys.reset();
    m_se->reset();

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

void WalkForwardSystem::_forceResetAll() {
    m_kdata_list.clear();
    m_cur_kdata = 0;
    m_cur_sys.reset();
    m_se->reset();

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

SystemPtr WalkForwardSystem::_clone() {
    WalkForwardSystem* p = new WalkForwardSystem();
    p->m_se = m_se->clone();
    return SystemPtr(p);
}

void WalkForwardSystem::syncDataFromSystem(const SYSPtr& sys, const KData& kdata, bool isMoment) {
    if (!isMoment) {
        const auto& sys_trade_list = sys->getTradeRecordList();
        std::copy(sys_trade_list.cbegin(), sys_trade_list.cend(), m_trade_list.begin());
    }

    m_buyRequest = sys->getBuyTradeRequest();
    m_sellRequest = sys->getSellTradeRequest();
    m_sellShortRequest = sys->getSellShortTradeRequest();
    m_buyShortRequest = sys->getBuyShortTradeRequest();

    m_pre_ev_valid = sys->m_pre_ev_valid;
    m_pre_cn_valid = sys->m_pre_cn_valid;

    m_buy_days = sys->m_buy_days;
    m_sell_short_days = sys->m_sell_short_days;
    m_lastTakeProfit = sys->m_lastTakeProfit;
    m_lastShortTakeProfit = sys->m_lastShortTakeProfit;

    auto dates = kdata.getDatetimeList();
    auto& ev = sys->getEV();
    if (ev) {
        for (const auto& date : dates) {
            if (ev->isValid(date)) {
                m_ev->_addValid(date);
            }
        }
    }

    auto& cn = sys->getCN();
    if (cn) {
        for (const auto& date : dates) {
            if (cn->isValid(date)) {
                m_cn->_addValid(date);
            }
        }
    }

    auto& sg = sys->getSG();
    if (sg) {
        auto buy_dates = sg->getBuySignal();
        for (const auto& date : buy_dates) {
            m_sg->_addBuySignal(date);
        }

        auto sell_dates = sg->getSellSignal();
        for (const auto& date : sell_dates) {
            m_sg->_addSellSignal(date);
        }
    }
}

void WalkForwardSystem::syncDataToSystem(const SYSPtr& sys) {
    sys->m_buyRequest = m_buyRequest;
    sys->m_sellRequest = m_sellRequest;
    sys->m_sellShortRequest = m_sellShortRequest;
    sys->m_buyShortRequest = m_buyShortRequest;

    sys->m_pre_ev_valid = m_pre_ev_valid;
    sys->m_pre_cn_valid = m_pre_cn_valid;

    sys->m_buy_days = m_buy_days;
    sys->m_sell_short_days = m_sell_short_days;
    sys->m_lastTakeProfit = m_lastTakeProfit;
    sys->m_lastShortTakeProfit = m_lastShortTakeProfit;
}

void WalkForwardSystem::readyForRun() {
    HKU_CHECK(m_tm, "Not setTradeManager! {}", name());
    HKU_CHECK(m_mm, "Not setMoneyManager! {}", name());
    HKU_CHECK(m_sg, "Not setSignal! {}", name());

    // 如果存在市场环境判断策略，则需要将默认的前一日市场有效标志置为false
    // 因为需要由市场环境判断策略全权判定市场是否有效
    if (m_ev)
        m_pre_ev_valid = false;

    if (m_cn) {
        m_cn->setTM(m_tm);
        m_cn->setSG(m_sg);
        m_pre_cn_valid = false;  // 默认的前一日市场有效标志置为false
    }

    m_mm->setTM(m_tm);
    if (m_pg)
        m_pg->setTM(m_tm);
    if (m_st)
        m_st->setTM(m_tm);
    if (m_tp)
        m_tp->setTM(m_tm);

    m_tm->setParam<bool>("support_borrow_cash", getParam<bool>("support_borrow_cash"));
    m_tm->setParam<bool>("support_borrow_stock", getParam<bool>("support_borrow_stock"));

    m_se->removeAll();
    for (auto& sys : m_candidate_sys_list) {
        sys->setTM(getTM()->clone());
    }
    m_se->addSystemList(m_candidate_sys_list);
    m_se->calculate(SystemList(), m_kdata.getQuery());
}

void WalkForwardSystem::run(const KData& kdata, bool reset, bool resetAll) {
    HKU_IF_RETURN(kdata.empty(), void());
    if (resetAll) {
        this->forceResetAll();
    } else if (reset) {
        this->reset();
    }

    HKU_DEBUG_IF_RETURN(m_calculated && m_kdata == kdata, void(), "Not need calculate.");

    m_kdata = kdata;
    readyForRun();

    OptimalSelector* se_ptr = dynamic_cast<OptimalSelector*>(m_se.get());
    const auto& run_ranges = se_ptr->getRunRanges();
    size_t run_ranges_len = run_ranges.size();
    HKU_IF_RETURN(run_ranges_len == 0, void());

    const KQuery& query = kdata.getQuery();
    const Stock& stock = kdata.getStock();

    for (size_t i = 0; i < run_ranges_len - 1; i++) {
        KQuery range_query = KQueryByDate(run_ranges[i].second, run_ranges[i].second, query.kType(),
                                          query.recoverType());
        m_kdata_list.emplace_back(stock.getKData(range_query));
    }

    m_kdata_list.emplace_back(stock.getKData(KQueryByDate(
      run_ranges[run_ranges_len - 1].second, run_ranges[run_ranges_len - 1].second + Seconds(1),
      query.kType(), query.recoverType())));

    for (auto& kdata : m_kdata_list) {
        if (kdata.empty()) {
            continue;
        }

        auto sw_list = m_se->getSelected(kdata[0].datetime);
        if (!sw_list.empty()) {
            auto& sys = sw_list.front().sys;
            sys->setTM(getTM());
            sys->readyForRun();
            syncDataToSystem(sys);
            sys->run(kdata);
            syncDataFromSystem(sys, kdata, false);
        }
    }

    m_calculated = true;
}

TradeRecord WalkForwardSystem::runMoment(const Datetime& datetime) {
    TradeRecord ret;
    auto sw_list = m_se->getSelected(datetime);
    HKU_IF_RETURN(sw_list.empty(), ret);

    auto& sys = sw_list.front().sys;
    if (sys != m_cur_sys) {
        m_cur_kdata++;
        m_cur_sys = sys;
        m_cur_sys->setTM(getTM());
        m_cur_sys->readyForRun();
        syncDataToSystem(m_cur_sys);
    }

    ret = m_cur_sys->runMoment(datetime);
    m_trade_list.push_back(ret);
    syncDataFromSystem(m_cur_sys, m_kdata_list[m_cur_kdata], true);
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->sellForceOnOpen(date, num, from);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, m_kdata_list[m_cur_kdata], true);
    }
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->sellForceOnClose(date, num, from);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, m_kdata_list[m_cur_kdata], true);
    }
    return ret;
}

void WalkForwardSystem::clearDelayBuyRequest() {
    if (m_cur_sys) {
        m_cur_sys->clearDelayBuyRequest();
        m_buyRequest.clear();
    }
}

bool WalkForwardSystem::haveDelaySellRequest() const {
    return m_sellRequest.valid;
}

TradeRecord WalkForwardSystem::pfProcessDelaySellRequest(const Datetime& date) {
    TradeRecord ret;
    if (m_cur_sys) {
        ret = m_cur_sys->pfProcessDelaySellRequest(date);
        m_trade_list.push_back(ret);
        syncDataFromSystem(m_cur_sys, m_kdata_list[m_cur_kdata], true);
    }
    return ret;
}

}  // namespace hku