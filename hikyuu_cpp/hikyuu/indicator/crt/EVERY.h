/*
 * EVERY.h
 *  
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_EVERY_H_
#define INDICATOR_CRT_EVERY_H_

#include "../Indicator.h"

namespace hku {

/**
 * 一直存在
 * @details
 * <pre>
 * 用法：EVERY (X,N) 表示条件X在N周期一直存在
 * 例如：EVERY(CLOSE>OPEN,10) 表示前10日内一直是阳线
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API EVERY(int n = 20);
Indicator EVERY(const Indicator& ind, int n = 20);

inline Indicator EVERY(const Indicator& ind, int n) {
    return EVERY(n)(ind);
}

} /* namespace */

#endif /* INDICATOR_CRT_EVERY_H_ */
