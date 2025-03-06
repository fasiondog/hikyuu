/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/environment/EnvironmentBase.h"

namespace hku {

/**
 * 两个市场环境判定相与，等效于两者的交集
 * @param ev1 市场环境判定1
 * @param ev2 市场环境判定2
 * @return AndCondition 实例指针
 */
HKU_API EnvironmentPtr operator&(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);

/**
 * 两个市场环境判定相或，等效于两者的并集
 * @param ev1 市场环境判定1
 * @param ev2 市场环境判定2
 * @return OrCondition 实例指针
 */
HKU_API EnvironmentPtr operator|(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);

HKU_API EnvironmentPtr operator+(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
HKU_API EnvironmentPtr operator-(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
HKU_API EnvironmentPtr operator*(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
HKU_API EnvironmentPtr operator/(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);

}  // namespace hku