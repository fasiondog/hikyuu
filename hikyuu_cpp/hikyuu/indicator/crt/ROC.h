/*
 * ROC.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ROC_H_
#define INDICATOR_CRT_ROC_H_

#include "../Indicator.h"

namespace hku {

/**
 * 变动率指标 ((price / prevPrice)-1)*100
 * @ingroup Indicator
 */
Indicator HKU_API ROC(int n =10);

Indicator ROC(const Indicator& ind, int n = 10) {
    return ROC(n)(ind);
}

}

#endif /* INDICATOR_CRT_ROC_H_ */
