/*
 * LAST.h
 *  
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_LAST_H_
#define INDICATOR_CRT_LAST_H_

#include "../Indicator.h"

namespace hku {

/**
 * 区间存在
 * @details
 * <pre>
 * 用法：LAST (X,M,N) 表示条件X在前M周期到前N周期存在
 * 例如：LAST(CLOSE>OPEN,10,5) 表示从前10日到前5日内一直阳线。若A为0,表示从第一天开始,B为0,表示到最后日止。
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API LAST(int m=10, int n=5);
Indicator LAST(const Indicator& ind, int m=10, int n=5);

inline Indicator LAST(const Indicator& ind, int m, int n) {
    return LAST(m, n)(ind);
}

} /* namespace */

#endif /* INDICATOR_CRT_LAST_H_ */
