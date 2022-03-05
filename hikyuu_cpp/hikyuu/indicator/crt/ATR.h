/*
 * ATR.h
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#pragma once
#ifndef INDICATOR_CRT_ATR_H_
#define INDICATOR_CRT_ATR_H_

#include "../Indicator.h"

namespace hku {

/**
 * 平均真实波幅(Average True Range)
 * @param n 计算均值的周期窗口，必须为大于1的整数
 * @ingroup Indicator
 */
Indicator HKU_API ATR(int n = 14);
Indicator HKU_API ATR(const IndParam& n);

/**
 * 平均真实波幅(Average True Range)
 * @param data 待计算的源数据
 * @param n 计算均值的周期窗口，必须为大于1的整数
 * @ingroup Indicator
 */
inline Indicator HKU_API ATR(const Indicator& data, int n = 14) {
    return ATR(n)(data);
}

inline Indicator HKU_API ATR(const Indicator& data, const IndParam& n) {
    return ATR(n)(data);
}

inline Indicator HKU_API ATR(const Indicator& data, const Indicator& n) {
    return ATR(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ATR_H_ */
