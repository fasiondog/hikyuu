/*
 * VARP.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_VARP_H_
#define INDICATOR_CRT_VARP_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算N周期内总体样本方差
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API VARP(int n = 10);
Indicator HKU_API VARP(const IndParam& n);

inline Indicator VARP(const Indicator& data, int n = 10) {
    return VARP(n)(data);
}

inline Indicator VARP(const Indicator& data, const IndParam& n) {
    return VARP(n)(data);
}

inline Indicator VARP(const Indicator& data, const Indicator& n) {
    return VARP(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_VARP_H_ */
