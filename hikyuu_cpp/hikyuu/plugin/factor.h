/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/factor/Factor.h"

namespace hku {

FactorImpPtr HKU_API createFactorImp(const string& name, const Indicator& formula,
                                     const KQuery::KType& ktype, const string& brief,
                                     const string& details, bool need_persist,
                                     const Datetime& start_date, const Block& block);

FactorList HKU_API getAllFactors();

void HKU_API updateAllFactorsValues(const KQuery::KType& ktype = KQuery::DAY);

}  // namespace hku