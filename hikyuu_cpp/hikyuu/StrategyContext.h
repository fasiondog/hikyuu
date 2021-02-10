/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#pragma once

#include "DataType.h"

namespace hku {

class HKU_API StrategyContext {
public:
    StrategyContext() = default;
    StrategyContext(const StrategyContext&) = default;

    virtual ~StrategyContext() = default;

    Datetime startDatetime() const {
        return m_startDatetime;
    }

    void startDatetime(const Datetime& d) {
        m_startDatetime = d;
    }

    void setStockCodeList(vector<string>&& stockList) {
        m_stockCodeList = std::move(stockList);
    }

    void setStockCodeList(const vector<string>& stockList);

    const vector<string>& getStockCodeList() const {
        return m_stockCodeList;
    }

private:
    Datetime m_startDatetime;
    vector<string> m_stockCodeList;
};

}  // namespace hku