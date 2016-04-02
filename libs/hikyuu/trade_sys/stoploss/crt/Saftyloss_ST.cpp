/*
 * Saftloss_ST.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include "../../../indicator/crt/SAFTYLOSS.h"
#include "Saftyloss_ST.h"

namespace hku {

StoplossPtr HKU_API Saftyloss_ST(int n1, int n2, double p) {
    Indicator ind = SAFTYLOSS(n1, n2, p);
    StoplossPtr result = StoplossPtr(
            new IndicatorStoploss("Saftyloss_ST", ind, "CLOSE"));
    return result;
}

} /* namespace */

