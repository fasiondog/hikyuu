/*
 * COUNT.h
 *
 *  Created on: 2019-3-25
 *      Author: fasiondog
 */

#pragma once
#ifndef COUNT_H_
#define COUNT_H_

#include "../Indicator.h"

namespace hku {

/**
 * 统计总数 统计满足条件的周期数。
 * @details
 * <pre>
 * 用法：COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。
 * 例如：COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数
 * </pre>
 * @param n 周期数
 * @ingroup Indicator
 */
Indicator HKU_API COUNT(int n = 20);
Indicator HKU_API COUNT(const IndParam& n);

/**
 * 统计总数 统计满足条件的周期数。
 * @details
 * <pre>
 * 用法：COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。
 * 例如：COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数
 * </pre>
 * @param ind 待统计指标
 * @param n 周期数
 * @ingroup Indicator
 */
inline Indicator HKU_API COUNT(const Indicator& ind, int n = 20) {
    return COUNT(n)(ind);
}

inline Indicator HKU_API COUNT(const Indicator& ind, const IndParam& n) {
    return COUNT(n)(ind);
}

inline Indicator HKU_API COUNT(const Indicator& ind, const Indicator& n) {
    return COUNT(IndParam(n))(ind);
}

}  // namespace hku

#endif /* CVAL_H_ */
