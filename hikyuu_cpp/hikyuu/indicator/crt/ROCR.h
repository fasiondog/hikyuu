/*
 * ROCR.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ROCR_H_
#define INDICATOR_CRT_ROCR_H_

#include "../Indicator.h"

namespace hku {

/**
 * 变动率指标 (price / prevPrice)
 * @ingroup Indicator
 */
Indicator HKU_API ROCR(int n =10);

Indicator ROCR(const Indicator& ind, int n = 10) {
    return ROCR(n)(ind);
}

}

#endif /* INDICATOR_CRT_ROCR_H_ */
