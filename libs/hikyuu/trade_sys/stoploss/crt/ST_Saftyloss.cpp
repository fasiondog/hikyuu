/*
 * Saftloss_ST.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/stoploss/crt/ST_Saftyloss.h>
#include "../../../indicator/crt/SAFTYLOSS.h"

namespace hku {

StoplossPtr HKU_API ST_Saftyloss(int n1, int n2, double p) {
    Indicator ind = SAFTYLOSS(n1, n2, p);
    IndicatorStoploss *result = new IndicatorStoploss(ind, "CLOSE");
    result->name("Saftyloss_ST");
    return StoplossPtr(result);
}

} /* namespace */

