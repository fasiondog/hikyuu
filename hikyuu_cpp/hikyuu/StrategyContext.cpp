/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include "StrategyContext.h"

namespace hku {

void StrategyContext::setStockCodeList(const vector<string>& stockList) {
    m_stockCodeList.resize(stockList.size());
    std::copy(stockList.begin(), stockList.end(), m_stockCodeList.begin());
}

}  // namespace hku