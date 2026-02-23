/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/factor/FactorSet.h"

namespace hku {

FactorImpPtr HKU_API createFactorImp(const string& name, const Indicator& formula,
                                     const KQuery::KType& ktype, const string& brief,
                                     const string& details, bool need_persist,
                                     const Datetime& start_date, const Block& block);

FactorList HKU_API getAllFactors();

FactorSetList HKU_API getAllFactorSets();

void HKU_API updateAllFactorsValues(const KQuery::KType& ktype = KQuery::DAY);

void HKU_API saveFactorSet(const FactorSet& set);
void HKU_API removeFactorSet(const string& name, const KQuery::KType& ktype);
FactorSet HKU_API getFactorSet(const string& name, const KQuery::KType& ktype);

}  // namespace hku