/*
 * TS_RANK.h
 *
 *  Created on: 2026年6月9日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TS_RANK_H_
#define INDICATOR_CRT_TS_RANK_H_

#include "../Indicator.h"

namespace hku {

/**
 * 时间序列排名，计算当前值在过去N个周期内的排名比例
 * @details
 * <pre>
 * 用法：TS_RANK(X,N)，表示X在过去N个周期内的排名（从1到N）除以N
 * 例如：TS_RANK(CLOSE,20)表示收盘价在过去20个周期内的排名比例
 *
 * Alpha101中的定义：
 * TS_RANK(x, n) = (rank of x in the last n observations) / n
 * 其中rank为升序排名，即较小的值排名靠前
 *
 * 实现说明：
 * 对于每个周期i，统计在[i-n+1, i]窗口内小于等于x[i]的元素个数count，
 * 则TS_RANK = count / n
 * </pre>
 * @param n 周期数
 * @ingroup Indicator
 */
Indicator HKU_API TS_RANK(int n = 20);
Indicator HKU_API TS_RANK(const IndParam& n);

/**
 * 时间序列排名，计算当前值在过去N个周期内的排名比例
 * @details
 * <pre>
 * 用法：TS_RANK(X,N)，表示X在过去N个周期内的排名（从1到N）除以N
 * 例如：TS_RANK(CLOSE,20)表示收盘价在过去20个周期内的排名比例
 * </pre>
 * @param ind 待计算的数据
 * @param n 周期数
 * @ingroup Indicator
 */
inline Indicator HKU_API TS_RANK(const Indicator& ind, int n = 20) {
    return TS_RANK(n)(ind);
}

inline Indicator HKU_API TS_RANK(const Indicator& ind, const IndParam& n) {
    return TS_RANK(n)(ind);
}

inline Indicator HKU_API TS_RANK(const Indicator& ind, const Indicator& n) {
    return TS_RANK(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_TS_RANK_H_ */