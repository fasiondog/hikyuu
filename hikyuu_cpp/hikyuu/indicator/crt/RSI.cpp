/*
 * RSI.h
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */

#include "hikyuu/utilities/Log.h"
#include "RSI.h"

namespace hku {

/**
 * 相对强弱指数
 * @ingroup Indicator
 */
Indicator HKU_API RSI(int n) {
    // Indicator data = Indicator();
    // Indicator diff = REF(data, 0) - REF(data, 1);
    Indicator diff = REF(0) - REF(1);
    Indicator u = IF(diff > 0, diff, 0);
    Indicator d = IF(diff < 0, (-1) * diff, 0);
    Indicator ema_u = EMA(u, n);
    Indicator ema_d = EMA(d, n);
    ema_d = IF(ema_d == 0.0, 1, ema_d);
    Indicator rs = ema_u / ema_d;
    Indicator _1 = CVAL(1);
    Indicator rsi = (_1 - _1 / (_1 + rs)) * CVAL(100);
    rsi.name("RSI");
    rsi.setParam<int>("n", n);
    return rsi;
}

Indicator HKU_API RSI(const Indicator& data, int n) {
    return RSI(n)(data)(data.getContext());
}

}  // namespace hku
