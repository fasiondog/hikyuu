/*
 * Flex_SG.cpp
 *
 *  Created on: 2016年4月11日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/EMA.h"
#include "Cross_SG.h"

namespace hku {

SignalPtr HKU_API Flex_SG(const Operand& op, int slow_n, const string& kpart) {
    SignalPtr sg = Cross_SG(op, OP(EMA(slow_n), op));
    sg->name("Flex_SG");
    return sg;
}

} /* namespace hku */


