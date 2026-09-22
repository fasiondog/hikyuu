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
 * Replace the given value, it is usually used to replace the Nan values
 * @param old_val the value to be replaced
 * @param new_val the replaced value
 * @param ignore_discard ignore discard; if the nan values are replaced, the new discard is set to 0
 * @ingroup Indicator
 */
Indicator HKU_API REPLACE(double old_val = Null<double>(), double new_val = 0.0,
                          bool ignore_discard = false);

inline Indicator REPLACE(const Indicator& ind, double old_val = Null<double>(),
                         double new_val = 0.0, bool ignore_discard = false) {
    return REPLACE(old_val, new_val, ignore_discard)(ind);
}

}  // namespace hku
