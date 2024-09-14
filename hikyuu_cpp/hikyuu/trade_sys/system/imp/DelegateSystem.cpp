/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "DelegateSystem.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::DelegateSystem)
#endif

namespace hku {

void DelegateSystem::_reset() {
    if (m_sys) {
        m_sys->reset();
    }

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

void DelegateSystem::_forceResetAll() {
    if (m_sys) {
        m_sys->forceResetAll();
    }

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();
}

SystemPtr DelegateSystem::_clone() {
    return m_sys ? make_shared<DelegateSystem>(m_sys->clone()) : make_shared<DelegateSystem>();
}

void DelegateSystem::run(const KData& kdata, bool reset, bool resetAll) {
    HKU_WARN_IF_RETURN(!m_sys, void(), "No delegated system is specified!");
    m_kdata = kdata;

    m_sys->setTM(getTM());
    m_sys->setMM(getMM());
    m_sys->setEV(getEV());
    m_sys->setCN(getCN());
    m_sys->setSG(getSG());
    m_sys->setST(getST());
    m_sys->setTP(getTP());
    m_sys->setPG(getPG());
    m_sys->setSP(getSP());

    m_sys->run(kdata, reset, resetAll);
    const auto& sys_trade_list = m_sys->getTradeRecordList();

    m_trade_list.clear();
    m_buyRequest.clear();
    m_sellRequest.clear();
    m_sellShortRequest.clear();
    m_buyShortRequest.clear();

    std::copy(sys_trade_list.cbegin(), sys_trade_list.cend(), m_trade_list.begin());
    m_buyRequest = m_sys->getBuyTradeRequest();
    m_sellRequest = m_sys->getSellTradeRequest();
    m_sellShortRequest = m_sys->getSellShortTradeRequest();
    m_buyShortRequest = m_sys->getBuyShortTradeRequest();
}

TradeRecord DelegateSystem::runMoment(const Datetime& datetime) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    ret = m_sys->runMoment(datetime);
    m_trade_list.push_back(ret);
    return ret;
}

TradeRecord DelegateSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    ret = m_sys->sellForceOnOpen(date, num, from);
    m_trade_list.push_back(ret);
    return ret;
}

TradeRecord DelegateSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    ret = m_sys->sellForceOnClose(date, num, from);
    m_trade_list.push_back(ret);
    return ret;
}

void DelegateSystem::clearDelayBuyRequest() {
    HKU_WARN_IF_RETURN(!m_sys, void(), "No delegated system is specified!");
    m_sys->clearDelayBuyRequest();
    m_buyRequest.clear();
}

bool DelegateSystem::haveDelaySellRequest() const {
    return m_sellRequest.valid;
}

TradeRecord DelegateSystem::pfProcessDelaySellRequest(const Datetime& date) {
    TradeRecord ret;
    HKU_WARN_IF_RETURN(!m_sys, ret, "No delegated system is specified!");
    ret = m_sys->pfProcessDelaySellRequest(date);
    m_trade_list.push_back(ret);
    return ret;
}

}  // namespace hku