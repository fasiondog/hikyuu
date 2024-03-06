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
 * 两个系统有效条件相与，等效于两者的交集
 * @param cond1 系统有效条件1
 * @param cond2 系统有效条件2
 * @return AndCondition 实例指针
 */
HKU_API ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2);

/**
 * 两个系统有效条件相或，等效于两者的并集
 * @param cond1 系统有效条件1
 * @param cond2 系统有效条件2
 * @return OrCondition 实例指针
 */
HKU_API ConditionPtr operator|(const ConditionPtr& cond1, const ConditionPtr& cond2);

HKU_API ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2);
HKU_API ConditionPtr operator-(const ConditionPtr& cond1, const ConditionPtr& cond2);
HKU_API ConditionPtr operator*(const ConditionPtr& cond1, const ConditionPtr& cond2);
HKU_API ConditionPtr operator/(const ConditionPtr& cond1, const ConditionPtr& cond2);

}  // namespace hku