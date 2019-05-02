/*
 * VAR.h
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_VAR_H_
#define INDICATOR_CRT_VAR_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算N周期内估算样本方差
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API VAR(int n = 10);
Indicator VAR(const Indicator& data, int n = 10);

inline Indicator VAR(const Indicator& data, int n) {
    return VAR(n)(data);
}

} /* namespace */


#endif /* INDICATOR_CRT_VAR_H_ */
