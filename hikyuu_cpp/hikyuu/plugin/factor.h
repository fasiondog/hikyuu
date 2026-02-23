/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/factor/FactorSet.h"

namespace hku {

/** 获取所有因子 */
FactorList HKU_API getAllFactors();

/** 获取所有因子集合 */
FactorSetList HKU_API getAllFactorSets();

/** 更新保存的所有因子数据至当前 */
void HKU_API updateAllFactorsValues(const KQuery::KType& ktype = KQuery::DAY);

/** 保存因子集合 */
void HKU_API saveFactorSet(const FactorSet& set);

/** 删除因子集合 */
void HKU_API removeFactorSet(const string& name, const KQuery::KType& ktype);

/** 获取因子集合 */
FactorSet HKU_API getFactorSet(const string& name, const KQuery::KType& ktype = KQuery::DAY);

FactorImpPtr HKU_API createFactorImp(const string& name, const Indicator& formula,
                                     const KQuery::KType& ktype, const string& brief,
                                     const string& details, bool need_persist,
                                     const Datetime& start_date, const Block& block);

}  // namespace hku