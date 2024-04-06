/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-06
 *      Author: fasiondog
 */

#include "KDATA.h"
#include "SUM.h"
#include "LIUTONGPAN.h"
#include "TURNOVER.h"

namespace hku {

// 不需要乘以 100，成交量已经是手数即100
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