/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SLOPE_H_
#define INDICATOR_CRT_SLOPE_H_

#include "../Indicator.h"

namespace hku {

Indicator HKU_API SLOPE(int n = 22);

inline Indicator SLOPE(const Indicator& ind, int n = 22) {
    return SLOPE(n)(ind);
}

}  // namespace hku

#endif
