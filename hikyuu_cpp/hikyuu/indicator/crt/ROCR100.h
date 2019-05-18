/*
 * ROCR100.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ROCR100_H_
#define INDICATOR_CRT_ROCR100_H_

#include "../Indicator.h"

namespace hku {

/**
 * 变动率指标 (price / prevPrice) * 100
 * @ingroup Indicator
 */
Indicator HKU_API ROCR100(int n =10);

Indicator ROCR100(const Indicator& ind, int n = 10) {
    return ROCR100(n)(ind);
}

}

#endif /* INDICATOR_CRT_ROCR100_H_ */
