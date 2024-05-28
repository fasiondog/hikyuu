/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

HKU_API SelectorPtr operator+(const SelectorPtr& se1, const SelectorPtr& se2);

HKU_API SelectorPtr operator+(const SelectorPtr& se, double value);

inline SelectorPtr operator+(double value, const SelectorPtr& se) {
    return se + value;
}

}  // namespace hku