/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API ZHBOND10(double default_val = 4.0);
Indicator HKU_API ZHBOND10(const DatetimeList& dates, double default_val = 4.0);
Indicator HKU_API ZHBOND10(const KData& k, double default_val = 4.0);

inline Indicator ZHBOND10(const Indicator& ind, double default_val = 4.0) {
    return ZHBOND10(default_val)(ind);
}

}  // namespace hku