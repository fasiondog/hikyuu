/*
 * ROCR.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ROCR_H_
#define INDICATOR_CRT_ROCR_H_

#include "../Indicator.h"

namespace hku {

/**
 * 变动率指标 (price / prevPrice),  N 日累积收益率 （包含本金）
 * @ingroup Indicator
 */
Indicator HKU_API ROCR(int n = 10);
Indicator HKU_API ROCR(const IndParam& n);

inline Indicator ROCR(const Indicator& ind, int n = 10) {
    return ROCR(n)(ind);
}

inline Indicator ROCR(const Indicator& ind, const IndParam& n) {
    return ROCR(n)(ind);
}

inline Indicator ROCR(const Indicator& ind, const Indicator& n) {
    return ROCR(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROCR_H_ */
