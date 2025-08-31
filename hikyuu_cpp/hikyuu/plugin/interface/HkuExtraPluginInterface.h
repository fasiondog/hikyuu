/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#pragma once

#include <arrow/api.h>
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class HkuExtraPluginInterface : public PluginBase {
public:
    HkuExtraPluginInterface() = default;
    virtual ~HkuExtraPluginInterface() = default;

    virtual void registerKTypeExtra(const string& ktype, const string& basetype, int32_t minutes,
                                    std::function<Datetime(const Datetime&)> getPhaseEnd) = 0;

    virtual void releaseKExtra() = 0;

    virtual bool isExtraKType(const string& ktype) = 0;

    virtual std::vector<string> getExtraKTypeList() = 0;

    virtual int32_t getKTypeExtraMinutes(const string& ktype) = 0;

    virtual KRecordList getExtraKRecordList(const Stock& stk, const KQuery& query) = 0;

    virtual size_t getStockExtraCount(const Stock& stk, const string& ktype) = 0;

    virtual bool getStockExtraIndexRange(const Stock& stk, const KQuery& query, size_t& out_start,
                                         size_t& out_end) = 0;

    virtual KDataImpPtr getKDataImp(const Stock& stk, const KQuery& query) = 0;

    virtual std::shared_ptr<arrow::Table> getIndicatorsViewParallel(const StockList& stks,
                                                                    const IndicatorList& inds,
                                                                    const KQuery& query,
                                                                    const string& market) = 0;

    virtual std::shared_ptr<arrow::Table> getIndicatorsViewParallel(
      const StockList& stks, const IndicatorList& inds, const Datetime& date, size_t cal_len,
      const KQuery::KType& ktype, const string& market) = 0;

    virtual std::shared_ptr<arrow::Table> getMarketViewParallel(const StockList& stks,
                                                                const Datetime& date,
                                                                const string& market) = 0;
};

}  // namespace hku
