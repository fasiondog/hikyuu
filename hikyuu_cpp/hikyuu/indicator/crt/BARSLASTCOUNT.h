/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BARSLASTCOUNT_H_
#define INDICATOR_CRT_BARSLASTCOUNT_H_

#include "CVAL.h"

namespace hku {

/**
 * 统计连续满足条件的周期数
 * @details
 * <pre>
 * 用法：BARSLASTCOUNT(X)，其中X为条件表达式。
 * 例如：BARSLASTCOUNT(CLOSE>OPEN)表示统计连续收阳的周期数
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSLASTCOUNT();

inline Indicator BARSLASTCOUNT(const Indicator& ind) {
    return BARSLASTCOUNT()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSLASTCOUNT_H_ */
