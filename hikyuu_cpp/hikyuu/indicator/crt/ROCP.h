/*
 * ROCP.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ROCP_H_
#define INDICATOR_CRT_ROCP_H_

#include "../Indicator.h"

namespace hku {

/**
 * 变动率指标 (price - prePrice) / prevPrice
 * @ingroup Indicator
 */
Indicator HKU_API ROCP(int n =10);

Indicator ROCP(const Indicator& ind, int n = 10) {
    return ROCP(n)(ind);
}

}

#endif /* INDICATOR_CRT_ROCP_H_ */
