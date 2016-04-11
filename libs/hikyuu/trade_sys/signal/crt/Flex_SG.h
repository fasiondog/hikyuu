/*
 * FLEX._SG.h
 *
 *  Created on: 2015年3月21日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_
#define TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_


#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

SignalPtr HKU_API Flex_SG(const Operand& op, int slow_n,
        const string& kpart="CLOSE");

} /* namespace hku */



#endif /* TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_ */
