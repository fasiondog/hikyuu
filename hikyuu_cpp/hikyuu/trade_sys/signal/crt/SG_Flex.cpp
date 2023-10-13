/*
 * SG_Flex.cpp
 *
 *  Created on: 2016年4月11日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/EMA.h"
#include "SG_Cross.h"

namespace hku {

SignalPtr HKU_API SG_Flex(const Indicator& op, int slow_n) {
    SignalPtr sg = SG_Cross(op, EMA(slow_n)(op));
    sg->name("SG_Flex");
    return sg;
}

} /* namespace hku */
