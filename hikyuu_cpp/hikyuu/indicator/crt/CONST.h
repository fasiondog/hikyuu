/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-31
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_CONST_H_
#define INDICATOR_CRT_CONST_H_

#include "CVAL.h"

namespace hku {

/**
 * 求绝对值
 * @ingroup Indicator
 */
Indicator HKU_API CONST();

inline Indicator CONST(const Indicator& ind) {
    return CONST()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_CONST_H_ */
