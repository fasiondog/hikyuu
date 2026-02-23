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

    virtual FactorImpPtr createFactorImp(const string& name, const Indicator& formula,
                                         const KQuery::KType& ktype, const string& brief,
                                         const string& details, bool need_persist,
                                         const Datetime& start_date, const Block& block) = 0;
    virtual FactorList getAllFactors() = 0;
    virtual FactorSetList getAllFactorSets() = 0;
    virtual void updateAllFactorsValues(const KQuery::KType& ktype) = 0;

    virtual FactorSet getFactorSet(const string& name, const KQuery::KType& ktype) = 0;
    virtual void saveFactorSet(const FactorSet& set) = 0;
    virtual void removeFactorSet(const string& name, const KQuery::KType& ktype) = 0;
};

}  // namespace hku
