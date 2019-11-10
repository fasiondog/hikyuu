/*
 * CN_OPLine.h
 *
 *  Created on: 2016年5月10日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_CONDITION_CRT_CN_OPLINE_H_
#define TRADE_SYS_CONDITION_CRT_CN_OPLINE_H_

#include "../../../indicator/Indicator.h"
#include "../ConditionBase.h"

namespace hku {

/**
 * 固定使用股票最小交易量进行交易，计算权益曲线的op值，当权益曲线高于op时，系统有效，否则无效。
 * @param op
 * @return
 */
CNPtr HKU_API CN_OPLine(const Indicator& op);

} /* namespace hku */

#endif /* TRADE_SYS_CONDITION_CRT_CN_OPLINE_H_ */
