/*
 * EV_TwoLine.h
 *
 *  Created on: 2016年5月17日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_
#define TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_

#include "../../../indicator/Operand.h"
#include "../EnvironmentBase.h"

namespace hku {

/**
 * 快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。
 * @param 快线指标
 * @param 慢线指标
 * @param market 市场名称，默认为"SH"
 * @return
 */
EVPtr HKU_API EV_TwoLine(const Operand& fast, const Operand& slow,
        const string& market);

} /* namespace hku */

#endif /* TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_ */
