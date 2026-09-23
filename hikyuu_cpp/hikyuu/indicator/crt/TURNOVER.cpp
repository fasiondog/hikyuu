/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-06
 *      Author: fasiondog
 */

#include "KDATA.h"
#include "SUM.h"
#include "LIUTONGPAN.h"

namespace hku {

// No need to multiply by 100, the trading volume is already in lots, i.e. 100
Indicator HKU_API TURNOVER(int n) {
    HKU_ASSERT(n >= 1);
    return n == 1 ? (VOL() / LIUTONGPAN()) : (SUM(VOL(), n) / SUM(LIUTONGPAN(), n));
}

Indicator HKU_API TURNOVER(const KData& kdata, int n) {
    HKU_ASSERT(n >= 1);
    return n == 1 ? (kdata.vol() / LIUTONGPAN(kdata))
                  : (SUM(kdata.vol(), n) / SUM(LIUTONGPAN(kdata), n));
}

}  // namespace hku