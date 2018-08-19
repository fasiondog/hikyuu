/*
 * SG_Flex.cpp
 *
 *  Created on: 2016年4月11日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/EMA.h"
#include "SG_Cross.h"

namespace hku {

SignalPtr HKU_API SG_Flex(const Operand& op, int slow_n, const string& kpart) {
    SignalPtr sg = SG_Cross(op, OP(OP(EMA(slow_n)), op), kpart);
    sg->name("SG_Flex");
    return sg;
}

} /* namespace hku */


