/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

#include "../crt/COST.h"
#include "../crt/DMA.h"
#include "../crt/HSL.h"

namespace hku {

Indicator HKU_API COST(double x) {
    Indicator ind = DMA(CLOSE() + (HIGH() - LOW()) * x / 100.0, HSL());
    ind.name("COST");
    ind.setParam<double>("x", x);
    return ind;
}

Indicator HKU_API COST(const KData& k, double x) {
    Indicator ind = COST(x);
    ind.setContext(k);
    return ind;
}

}  // namespace hku