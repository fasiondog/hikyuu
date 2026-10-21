/*
 * Saftloss_ST.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include <cmath>
#include "ST_Saftyloss.h"
#include "../../../indicator/crt/SAFTYLOSS.h"
#include "../../../indicator/crt/KDATA.h"

namespace hku {

StoplossPtr HKU_API ST_Saftyloss(int n1, int n2, double p) {
    HKU_CHECK(std::isfinite(p) && p > 0.0, "p must be finite and greater than 0, but got {}", p);
    Indicator op = SAFTYLOSS(CLOSE(), n1, n2, p);
    auto result = make_shared<IndicatorStoploss>(op);
    result->name("Saftyloss_ST");
    return result;
}

}  // namespace hku
