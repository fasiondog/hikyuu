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
 * Count the number of the consecutive periods satisfying the condition
 * @details
 * <pre>
 * Usage: BARSLASTCOUNT(X), where X is a condition expression.
 * For example: BARSLASTCOUNT(CLOSE>OPEN) counts the number of the consecutive periods closing up
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSLASTCOUNT();

inline Indicator BARSLASTCOUNT(const Indicator& ind) {
    return BARSLASTCOUNT()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSLASTCOUNT_H_ */
