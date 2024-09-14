/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "DelegateSystem.h"

namespace hku {

void DelegateSystem::_reset() {
    if (m_sys) {
        m_sys->reset();
    }
}

void DelegateSystem::_forceResetAll() {
    if (m_sys) {
        m_sys->forceResetAll();
    }
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
}

TradeRecord DelegateSystem::runMoment(const Datetime& datetime) {
    HKU_WARN_IF_RETURN(!m_sys, TradeRecord(), "No delegated system is specified!");
    return m_sys->runMoment(datetime);
}

TradeRecord DelegateSystem::sellForceOnOpen(const Datetime& date, double num, Part from) {
    HKU_WARN_IF_RETURN(!m_sys, TradeRecord(), "No delegated system is specified!");
    return m_sys->sellForceOnOpen(date, num, from);
}

TradeRecord DelegateSystem::sellForceOnClose(const Datetime& date, double num, Part from) {
    HKU_WARN_IF_RETURN(!m_sys, TradeRecord(), "No delegated system is specified!");
    return m_sys->sellForceOnClose(date, num, from);
}

void DelegateSystem::clearDelayBuyRequest() {
    HKU_WARN_IF_RETURN(!m_sys, void(), "No delegated system is specified!");
    m_sys->clearDelayBuyRequest();
}

bool DelegateSystem::haveDelaySellRequest() const {
    HKU_WARN_IF_RETURN(!m_sys, false, "No delegated system is specified!");
    return m_sys->haveDelaySellRequest();
}

TradeRecord DelegateSystem::pfProcessDelaySellRequest(const Datetime& date) {
    HKU_WARN_IF_RETURN(!m_sys, TradeRecord(), "No delegated system is specified!");
    return m_sys->pfProcessDelaySellRequest(date);
}

}  // namespace hku