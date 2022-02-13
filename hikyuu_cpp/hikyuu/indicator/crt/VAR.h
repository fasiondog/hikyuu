/*
 * VAR.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
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
Indicator HKU_API VAR(const IndParam& n);

inline Indicator VAR(const Indicator& data, int n = 10) {
    return VAR(n)(data);
}

inline Indicator VAR(const Indicator& data, const IndParam& n) {
    return VAR(n)(data);
}

inline Indicator VAR(const Indicator& data, const Indicator& n) {
    return VAR(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_VAR_H_ */
