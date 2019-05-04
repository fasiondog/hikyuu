/*
 * BARSSINCE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_BARSSINCE_H_
#define INDICATOR_CRT_BARSSINCE_H_

#include "CVAL.h"

namespace hku {

/**
 * 上一次条件成立位置, 上一次条件成立到当前的周期数。
 * @details
 * <pre>
 * 用法：BARSLAST(X): 上一次 X 不为 0 到现在的天数。
 * 例如：BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) 表示上一个涨停板到当前的周期数。
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSSINCE();
Indicator BARSSINCE(price_t);
Indicator BARSSINCE(const Indicator& ind);

inline Indicator BARSSINCE(const Indicator& ind) {
    return BARSSINCE()(ind);
}

inline Indicator BARSSINCE(price_t val) {
    return BARSSINCE(CVAL(val));
}

}

#endif /* INDICATOR_CRT_BARSSINCE_H_ */
