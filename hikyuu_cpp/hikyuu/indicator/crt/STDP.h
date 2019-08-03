/*
 * STD.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_STDP_H_
#define INDICATOR_CRT_STDP_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算N周期内总体标准差
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API STDP(int n = 10);
Indicator STDP(const Indicator& data, int n = 10);

inline Indicator STDP(const Indicator& data, int n) {
    return STDP(n)(data);
}

} /* namespace */


#endif /* INDICATOR_CRT_STDP_H_ */
