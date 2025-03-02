/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_KALMAN_H_
#define INDICATOR_CRT_KALMAN_H_

#include "CVAL.h"

namespace hku {

/**
 * 卡尔曼滤波
 * @param q 噪声方差
 * @param r 测量噪声方差
 * @ingroup Indicator
 */
Indicator HKU_API KALMAN(double q = 0.01, double r = 0.1);

inline Indicator KALMAN(const Indicator& ind, double q = 0.01, double r = 0.1) {
    return KALMAN(q, r)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_KALMAN_H_ */
