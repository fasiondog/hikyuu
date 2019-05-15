/*
 * SMA.h
 *
 *  Created on: 2015年2月16日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_SMA_H_
#define INDICATOR_CRT_SMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 求移动平均
 * @details
 * <pre>
 * 用法：若Y=SMA(X,N,M) 则 Y=[M*X+(N-M)*Y')/N,其中Y'表示上一周期Y值
 * </pre>
 * @param data 待计算的数据
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @param m 系数
 * @ingroup Indicator
 */
Indicator HKU_API SMA(int n = 22, double m = 2.0);
Indicator SMA(const Indicator& data, int n = 22, double m = 2.0);

inline Indicator SMA(const Indicator& ind, int n, double m){
    return SMA(n, m)(ind);
}


} /* namespace */



#endif /* INDICATOR_CRT_SMA_H_ */
