/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API RSRS_BULL(int n = 20, int m = 60);
Indicator HKU_API RSRS_BULL(const KData& kdata, int n = 20, int m = 60);

}  // namespace hku