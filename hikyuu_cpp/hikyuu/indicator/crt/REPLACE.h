/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-12
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 替换指定数值，通常用于替换 Nan 值
 * @param old_val 要替换的数值
 * @param new_val 替换后的数值
 * @param ignore_discard 忽略 discard，如果替换 nan 值，新discard将被设置为 0
 * @ingroup Indicator
 */
Indicator HKU_API REPLACE(double old_val = Null<double>(), double new_val = 0.0,
                          bool ignore_discard = false);

inline Indicator REPLACE(const Indicator& ind, double old_val = Null<double>(),
                         double new_val = 0.0, bool ignore_discard = false) {
    return REPLACE(old_val, new_val, ignore_discard)(ind);
}

}  // namespace hku
