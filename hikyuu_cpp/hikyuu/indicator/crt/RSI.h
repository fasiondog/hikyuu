/*
 * RSI.h
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */
#pragma once
#ifndef INDICATOR_CRT_RSI_H_
#define INDICATOR_CRT_RSI_H_

#include "../Indicator.h"
#include "REF.h"
#include "EMA.h"
#include "CVAL.h"

namespace hku {

/**
 * 相对强弱指数
 * @ingroup Indicator
 */
Indicator RSI(int n = 14) {
    Indicator diff = REF(0) - REF(1);
    Indicator u = IF(diff > 0, diff, 0);
    Indicator d = IF(diff < 0, (-1) * diff, 0);
    Indicator ema_u = EMA(u, n);
    Indicator ema_d = EMA(d, n);
    Indicator rs = ema_u / ema_d;
    Indicator _1 = CVAL(1);
    Indicator rsi = (_1 - _1 / (_1 + rs)) * CVAL(100);
    rsi.name("RSI");
    rsi.setParam<int>("n", n);
    return rsi;
}

}  // namespace hku

#endif /* INDICATOR_CRT_ABS_H_ */
