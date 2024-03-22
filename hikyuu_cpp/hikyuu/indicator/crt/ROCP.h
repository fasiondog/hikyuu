/*
 * ROCP.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ROCP_H_
#define INDICATOR_CRT_ROCP_H_

#include "../Indicator.h"

namespace hku {

/**
 * 变动率指标 (price - prePrice) / prevPrice   N 日收益率 （本金之外的盈利）
 * @ingroup Indicator
 */
Indicator HKU_API ROCP(int n = 10);
Indicator HKU_API ROCP(const IndParam& n);

inline Indicator ROCP(const Indicator& ind, int n = 10) {
    return ROCP(n)(ind);
}

inline Indicator ROCP(const Indicator& ind, const IndParam& n) {
    return ROCP(n)(ind);
}

inline Indicator ROCP(const Indicator& ind, const Indicator& n) {
    return ROCP(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROCP_H_ */
