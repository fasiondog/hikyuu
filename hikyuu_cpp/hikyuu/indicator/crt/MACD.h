/*
 * MACD.h
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef MACD_H_
#define MACD_H_

#include "../Indicator.h"

namespace hku {

/**
 * MACD moving average convergence / divergence
 * @param n1 short-term EMA time window, 12 by default
 * @param n2 long-term EMA time window, 26 by default
 * @param n3 EMA smoothing time window of (short-term EMA - long-term EMA), 9 by default
 * @return
 * <pre>
 * MACD BAR: MACD histogram, i.e. MACD fast line - MACD slow line
 * DIFF: fast line, i.e. (short-term EMA - long-term EMA)
 * DEA: slow line, i.e. the n3-period EMA smoothing of the fast line
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API MACD(int n1 = 12, int n2 = 26, int n3 = 9);
Indicator HKU_API MACD(const IndParam& n1, const IndParam& n2, const IndParam& n3);

/**
 * MACD moving average convergence / divergence
 * @param data the data to be calculated
 * @param n1 short-term EMA time window, 12 by default
 * @param n2 long-term EMA time window, 26 by default
 * @param n3 EMA smoothing time window of (short-term EMA - long-term EMA), 9 by default
 * @return
 * <pre>
 * MACD BAR: MACD histogram, i.e. MACD fast line - MACD slow line
 * DIFF: fast line, i.e. (short-term EMA - long-term EMA)
 * DEA: slow line, i.e. the n3-period EMA smoothing of the fast line
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MACD(const Indicator& data, int n1 = 12, int n2 = 26, int n3 = 9) {
    return MACD(n1, n2, n3)(data);
}

inline Indicator MACD(const Indicator& data, const IndParam& n1, const IndParam& n2,
                      const IndParam& n3) {
    return MACD(n1, n2, n3)(data);
}

inline Indicator MACD(const Indicator& data, const Indicator& n1, const Indicator& n2,
                      const Indicator& n3) {
    return MACD(IndParam(n1), IndParam(n2), IndParam(n3))(data);
}

}  // namespace hku

#endif /* MACD_H_ */
