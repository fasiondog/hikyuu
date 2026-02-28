/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/data_driver/KDataDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/utilities/plugin/PluginBase.h"
#include "hikyuu/factor/FactorSet.h"

namespace hku {

class DataDriverPluginInterface : public PluginBase {
public:
    DataDriverPluginInterface() = default;
    virtual ~DataDriverPluginInterface() = default;

    virtual KDataDriverPtr getKDataDriver() = 0;
    virtual BlockInfoDriverPtr getBlockInfoDriver() = 0;
    virtual BaseInfoDriverPtr getBaseInfoDriver() = 0;

    virtual FactorList getAllFactors() = 0;
    virtual FactorSetList getAllFactorSets() = 0;
    virtual void updateAllFactorsValues(const KQuery::KType& ktype) = 0;

    virtual Factor getFactor(const string& name, const KQuery::KType& ktype) = 0;
    virtual void saveFactor(const Factor& factor) = 0;
    virtual void removeFactor(const string& name, const KQuery::KType& ktype) = 0;
    virtual void saveSpecialFactorValues(const Factor& factor, const Stock& stock,
                                         const DatetimeList& dates, const PriceList& values,
                                         bool replace) = 0;

    virtual FactorSet getFactorSet(const string& name, const KQuery::KType& ktype) = 0;
    virtual void saveFactorSet(const FactorSet& set) = 0;
    virtual void removeFactorSet(const string& name, const KQuery::KType& ktype) = 0;

    virtual IndicatorList getValues(const Factor& factor, const StockList& stocks,
                                    const KQuery& query, bool align, bool fill_null, bool tovalue,
                                    const DatetimeList& align_dates) = 0;
    virtual vector<IndicatorList> getValues(const FactorSet& factor_set, const StockList& stocks,
                                            const KQuery& query, bool align, bool fill_null,
                                            bool tovalue, const DatetimeList& align_dates) = 0;
};

}  // namespace hku
