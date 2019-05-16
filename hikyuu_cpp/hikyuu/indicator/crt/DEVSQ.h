/*
 * DEVSQ.h
 * 
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_DEVSQ_H_
#define INDICATOR_CRT_DEVSQ_H_

#include "../Indicator.h"

namespace hku {

/**
 * 数据偏差平方和，求X的N日数据偏差平方和
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API DEVSQ(int n = 10);
Indicator DEVSQ(const Indicator& data, int n = 10);

inline Indicator DEVSQ(const Indicator& data, int n) {
    return DEVSQ(n)(data);
}

} /* namespace */


#endif /* INDICATOR_CRT_DEVSQ_H_ */
