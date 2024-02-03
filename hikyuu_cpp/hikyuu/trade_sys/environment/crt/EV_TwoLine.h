/*
 * EV_TwoLine.h
 *
 *  Created on: 2016年5月17日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_
#define TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_

#include "../../../indicator/Indicator.h"
#include "../EnvironmentBase.h"

namespace hku {

/**
 * 快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。
 * @param fast 快线指标
 * @param slow 慢线指标
 * @param market 市场名称，默认为"SH"
 * @return
 */
EVPtr HKU_API EV_TwoLine(const Indicator& fast, const Indicator& slow, const string& market = "SH");

} /* namespace hku */

#endif /* TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_ */
