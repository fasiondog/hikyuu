/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Yield of the 10-year Chinese treasury bond
 * @note The date in the context is used as the reference date with priority
 * @param default_val if the given date is earlier than the existing earliest treasury bond data,
 *                    this given default value is used
 * @return Indicator
 */
Indicator HKU_API ZHBOND10(double default_val = 4.0);
Indicator HKU_API ZHBOND10(const DatetimeList& dates, double default_val = 4.0);
Indicator HKU_API ZHBOND10(const KData& k, double default_val = 4.0);
inline Indicator ZHBOND10(const Indicator& ind, double default_val = 4.0) {
    return ZHBOND10(default_val)(ind);
}

}  // namespace hku