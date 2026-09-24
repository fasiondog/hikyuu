/*
 * CN_OPLine.h
 *
 *  Created on: 2016-5-10
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_CONDITION_CRT_CN_OPLINE_H_
#define TRADE_SYS_CONDITION_CRT_CN_OPLINE_H_

#include "../../../indicator/Indicator.h"
#include "../ConditionBase.h"

namespace hku {

/**
 * It always trades with the minimum trade quantity of the stock and calculates the op value of the
 * equity curve; the system is valid when the equity curve is higher than op, otherwise it is
 * invalid.
 * @param op
 * @return
 */
CNPtr HKU_API CN_OPLine(const Indicator& op);

} /* namespace hku */

#endif /* TRADE_SYS_CONDITION_CRT_CN_OPLINE_H_ */
