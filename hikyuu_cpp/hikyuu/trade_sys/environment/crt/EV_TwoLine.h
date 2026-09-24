/*
 * EV_TwoLine.h
 *
 *  Created on: 2016-5-17
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_
#define TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_

#include "../../../indicator/Indicator.h"
#include "../EnvironmentBase.h"

namespace hku {

/**
 * Fast and slow line strategy: the market is valid when the fast line of the market index
 * is greater than the slow line, otherwise it is invalid.
 * @param fast fast line indicator
 * @param slow slow line indicator
 * @param market market name, "SH" by default
 * @return
 */
EVPtr HKU_API EV_TwoLine(const Indicator& fast, const Indicator& slow, const string& market = "SH");

} /* namespace hku */

#endif /* TRADE_SYS_ENVIRONMENT_CRT_EV_TWOLINE_H_ */
