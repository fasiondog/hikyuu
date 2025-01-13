/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISINFA_H_
#define INDICATOR_CRT_ISINFA_H_

#include "CVAL.h"

namespace hku {

/**
 * 是否为负无穷大 （正无穷大使用 ISINF)
 * @ingroup Indicator
 */
Indicator HKU_API ISINFA();

inline Indicator ISINFA(const Indicator& ind) {
    return ISINFA()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISINFA_H_ */
