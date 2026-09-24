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
 * The AND of two market environments, equivalent to the intersection of the two
 * @param ev1 market environment 1
 * @param ev2 market environment 2
 * @return the AndCondition instance pointer
 */
HKU_API EnvironmentPtr operator&(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);

/**
 * The OR of two market environments, equivalent to the union of the two
 * @param ev1 market environment 1
 * @param ev2 market environment 2
 * @return the OrCondition instance pointer
 */
HKU_API EnvironmentPtr operator|(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);

HKU_API EnvironmentPtr operator+(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
HKU_API EnvironmentPtr operator-(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
HKU_API EnvironmentPtr operator*(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
HKU_API EnvironmentPtr operator/(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);

}  // namespace hku