/*
 * FILTER.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_FILTER_H_
#define INDICATOR_CRT_FILTER_H_

#include "../Indicator.h"

namespace hku {

/**
 * 信号过滤, 过滤连续出现的信号。
 * @details
 * <pre>
 * 用法：FILTER(X,N): X 满足条件后，删除其后 N 周期内的数据置为 0
 * 例如：FILTER(CLOSE>OPEN,5) 查找阳线，5天内再次出现的阳线不被记录在内。
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API FILTER(int n = 5);
Indicator HKU_API FILTER(const IndParam& n);

inline Indicator FILTER(const Indicator& ind, int n = 5) {
    return FILTER(n)(ind);
}

inline Indicator FILTER(const Indicator& ind, const IndParam& n) {
    return FILTER(n)(ind);
}

inline Indicator FILTER(const Indicator& ind, const Indicator& n) {
    return FILTER(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_FILTER_H_ */
