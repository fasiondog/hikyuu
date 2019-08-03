/*
 * ST_Indicator.h
 *
 *  Created on: 2016年4月19日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_STOPLOSS_CRT_ST_INDICATOR_H_
#define TRADE_SYS_STOPLOSS_CRT_ST_INDICATOR_H_

#include "../../../indicator/Indicator.h"
#include "../StoplossBase.h"

namespace hku {

StoplossPtr HKU_API ST_Indicator(const Indicator& op, const string& kpart = "CLOSE");

} /* namespace hku */

#endif /* TRADE_SYS_STOPLOSS_CRT_ST_INDICATOR_H_ */
