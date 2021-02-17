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

bool StrategyContext::isAll() const {
    return std::find_if(m_stockCodeList.begin(), m_stockCodeList.end(), [](string val) {
               to_upper(val);
               return val == "ALL";
           }) != m_stockCodeList.end();
}

}  // namespace hku