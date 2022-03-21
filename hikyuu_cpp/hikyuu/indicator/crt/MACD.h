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
 * MACD平滑异同移动平均线
 * @param n1 短期EMA时间窗，默认12
 * @param n2 长期EMA时间窗，默认26
 * @param n3 （短期EMA-长期EMA）EMA平滑时间窗，默认9
 * @return
 * <pre>
 * MACD BAR： MACD直柱，即MACD快线－MACD慢线
 * DIFF: 快线,即（短期EMA-长期EMA）
 * DEA: 慢线，即快线的n3周期EMA平滑
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API MACD(int n1 = 12, int n2 = 26, int n3 = 9);
Indicator HKU_API MACD(const IndParam& n1, const IndParam& n2, const IndParam& n3);

/**
 * MACD平滑异同移动平均线
 * @param data 待计算数据
 * @param n1 短期EMA时间窗，默认12
 * @param n2 长期EMA时间窗，默认26
 * @param n3 （短期EMA-长期EMA）EMA平滑时间窗，默认9
 * @return
 * <pre>
 * MACD BAR： MACD直柱，即MACD快线－MACD慢线
 * DIFF: 快线,即（短期EMA-长期EMA）
 * DEA: 慢线，即快线的n3周期EMA平滑
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
