/*
 * SUMBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SUMBARS_H_
#define INDICATOR_CRT_SUMBARS_H_

#include "../Indicator.h"

namespace hku {

/**
 * 累加到指定周期数, 向前累加到指定值到现在的周期数
 * @details
 * <pre>
 * 用法：SUMBARS(X,A):将X向前累加直到大于等于A,返回这个区间的周期数
 * 例如：SUMBARS(VOL,CAPITAL)求完全换手到现在的周期数
 * </pre>
 * @note discard 语义(标量参数 vs 序列参数存在差异):
 *  - 标量参数 `SUMBARS(ind, double)`: 若某位累加到序列最左端仍 < a, 整段被标为 discard
 *    (静态全局优化, 因标量 a 单调不可达时此前各位更不可达).
 *  - 序列参数 `SUMBARS(ind, IndParam)`: 不可达位逐位写 NaN, 但不推进 discard
 *    (动态 a 序列非单调, a[i] 不可达不代表 a[i+1] 不可达, 推进会抹杀后续可计算位).
 *  即动态路径不保证"discard 之后全为有效数值". 下游应按 `std::isnan` 处理,
 *  不应假设"discard 之后皆有效".
 * @ingroup Indicator
 */
Indicator HKU_API SUMBARS(double a);
Indicator HKU_API SUMBARS(const IndParam& a);

inline Indicator SUMBARS(const Indicator& ind, double a) {
    return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const IndParam& a) {
    return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const Indicator& a) {
    return SUMBARS(IndParam(a))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SUMBARS_H_ */
