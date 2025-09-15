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

class HkuViewsPluginInterface : public PluginBase {
public:
    HkuViewsPluginInterface() = default;
    virtual ~HkuViewsPluginInterface() = default;

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
