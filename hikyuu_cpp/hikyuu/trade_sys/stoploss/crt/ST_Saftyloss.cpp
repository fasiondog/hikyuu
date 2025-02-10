/*
 * Saftloss_ST.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include "ST_Saftyloss.h"
#include "../../../indicator/crt/SAFTYLOSS.h"

namespace hku {

StoplossPtr HKU_API ST_Saftyloss(int n1, int n2, double p) {
    Indicator op = SAFTYLOSS(n1, n2, p);
    auto result = make_shared<IndicatorStoploss>(op);
    result->name("Saftyloss_ST");
    return result;
}

}  // namespace hku
