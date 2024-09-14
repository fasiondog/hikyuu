/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/selector/crt/SE_Optimal.h"
#include "WalkForwardSystem.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WalkForwardSystem)
#endif

namespace hku {

WalkForwardSystem::WalkForwardSystem() : System("SYS_Optimal"), m_se(SE_Optimal()) {}

WalkForwardSystem::WalkForwardSystem(const SystemList& candidate_sys_list) : WalkForwardSystem() {
    m_se->addSystemList(candidate_sys_list);
}

void WalkForwardSystem::_reset() {
    m_se->reset();

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

void WalkForwardSystem::_forceResetAll() {
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

void WalkForwardSystem::run(const KData& kdata, bool reset, bool resetAll) {
    m_kdata = kdata;

    m_se->reset();

    m_se->calculate(SystemList(), kdata.getQuery());

    auto dates = kdata.getDatetimeList();
    SystemWeightList sw_list;
    SystemPtr sys;
    for (const auto& date : dates) {
        sw_list = m_se->getSelected(date);
        if (!sw_list.empty()) {
            sys = sw_list.front().sys;
        }
    }

    // m_sys->setTM(getTM());
    // m_sys->setMM(getMM());
    // m_sys->setEV(getEV());
    // m_sys->setCN(getCN());
    // m_sys->setSG(getSG());
    // m_sys->setST(getST());
    // m_sys->setTP(getTP());
    // m_sys->setPG(getPG());
    // m_sys->setSP(getSP());

    // m_sys->run(kdata, reset, resetAll);
    // const auto& sys_trade_list = m_sys->getTradeRecordList();

    // m_trade_list.clear();
    // m_buyRequest.clear();
    // m_sellRequest.clear();
    // m_sellShortRequest.clear();
    // m_buyShortRequest.clear();

    // std::copy(sys_trade_list.cbegin(), sys_trade_list.cend(), m_trade_list.begin());
    // m_buyRequest = m_sys->getBuyTradeRequest();
    // m_sellRequest = m_sys->getSellTradeRequest();
    // m_sellShortRequest = m_sys->getSellShortTradeRequest();
    // m_buyShortRequest = m_sys->getBuyShortTradeRequest();
}

TradeRecord WalkForwardSystem::runMoment(const Datetime& datetime) {
    TradeRecord ret;
    // HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    // ret = m_sys->runMoment(datetime);
    // m_trade_list.push_back(ret);
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    // HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    // ret = m_sys->sellForceOnOpen(date, num, from);
    // m_trade_list.push_back(ret);
    return ret;
}

TradeRecord WalkForwardSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    // HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    // ret = m_sys->sellForceOnClose(date, num, from);
    // m_trade_list.push_back(ret);
    return ret;
}

void WalkForwardSystem::clearDelayBuyRequest() {
    // HKU_WARN_IF_RETURN(!m_sys, void(), "No delegated system is specified!");
    // m_sys->clearDelayBuyRequest();
    // m_buyRequest.clear();
}

bool WalkForwardSystem::haveDelaySellRequest() const {
    return m_sellRequest.valid;
}

TradeRecord WalkForwardSystem::pfProcessDelaySellRequest(const Datetime& date) {
    TradeRecord ret;
    // HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    // ret = m_sys->pfProcessDelaySellRequest(date);
    // m_trade_list.push_back(ret);
    return ret;
}

}  // namespace hku