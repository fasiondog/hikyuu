/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#pragma once

#include "DataType.h"
#include "KQuery.h"

namespace hku {

class HKU_API StrategyContext {
public:
    StrategyContext() {}
    StrategyContext(const StrategyContext&) = default;

    explicit StrategyContext(const vector<string>& stockCodeList)
    : m_stockCodeList(stockCodeList) {}
    explicit StrategyContext(vector<string>&& stockCodeList)
    : m_stockCodeList(std::move(stockCodeList)) {}

    virtual ~StrategyContext() = default;

    bool isAll() const;

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

    void setKTypeList(const vector<KQuery::KType>& ktypeList);

    const vector<KQuery::KType>& getKTypeList() const {
        return m_ktypeList;
    }

private:
    Datetime m_startDatetime;
    vector<string> m_stockCodeList;
    vector<KQuery::KType> m_ktypeList;
};

}  // namespace hku