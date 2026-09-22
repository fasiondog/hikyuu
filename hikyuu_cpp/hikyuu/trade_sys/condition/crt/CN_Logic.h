/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/condition/ConditionBase.h"

namespace hku {

/**
 * The AND of two system valid conditions, equivalent to the intersection of the two
 * @param cond1 system valid condition 1
 * @param cond2 system valid condition 2
 * @return the AndCondition instance pointer
 */
HKU_API ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2);

/**
 * The OR of two system valid conditions, equivalent to the union of the two
 * @param cond1 system valid condition 1
 * @param cond2 system valid condition 2
 * @return the OrCondition instance pointer
 */
HKU_API ConditionPtr operator|(const ConditionPtr& cond1, const ConditionPtr& cond2);

HKU_API ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2);
HKU_API ConditionPtr operator-(const ConditionPtr& cond1, const ConditionPtr& cond2);
HKU_API ConditionPtr operator*(const ConditionPtr& cond1, const ConditionPtr& cond2);
HKU_API ConditionPtr operator/(const ConditionPtr& cond1, const ConditionPtr& cond2);

}  // namespace hku