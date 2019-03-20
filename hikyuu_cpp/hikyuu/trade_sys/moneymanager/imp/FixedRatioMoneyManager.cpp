/*
 * FixedRatioMoneyManager.cpp
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "FixedRatioMoneyManager.h"

namespace hku {

FixedRatioMoneyManager::FixedRatioMoneyManager()
: MoneyManagerBase("MM_FixedRadio") ,
  m_current_num(1),
  m_pre_cash(0.0) {
    setParam<double>("delta", 1000.00);
}

FixedRatioMoneyManager::~FixedRatioMoneyManager() {

}

void FixedRatioMoneyManager::_reset() {
    m_current_num = 1;
    m_pre_cash = 0.0;
}

MoneyManagerPtr FixedRatioMoneyManager::_clone() {
    FixedRatioMoneyManager *p = new FixedRatioMoneyManager();
    p->m_current_num = m_current_num;
    p->m_pre_cash = m_pre_cash;
    return MoneyManagerPtr(p);
}

size_t FixedRatioMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
    double delta = getParam<double>("delta");
    if (m_pre_cash == 0.0) {
        m_pre_cash = m_tm->initCash();
        m_current_num = (m_pre_cash / delta) / stock.minTradeNumber();
    }

    price_t level = m_pre_cash + m_current_num * getParam<double>("delta");
    price_t current_cash = m_tm->currentCash();

    if (current_cash > level) {
        m_current_num++;

    } else if (current_cash < level) {
        m_current_num--;
        if (m_current_num < 1) {
            m_current_num = 1;
        }
    }

    m_pre_cash = current_cash;

    return m_current_num * stock.minTradeNumber();
}

} /* namespace hku */
