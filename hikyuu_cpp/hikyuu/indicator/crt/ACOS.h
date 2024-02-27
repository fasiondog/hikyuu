/*
 * ACOS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ACOS_H_
#define INDICATOR_CRT_ACOS_H_

#include "CVAL.h"

namespace hku {

/**
 * 余弦值
 * @ingroup Indicator
 */
Indicator HKU_API ACOS();
Indicator ACOS(Indicator::value_t);
Indicator ACOS(const Indicator& ind);

inline Indicator ACOS(const Indicator& ind) {
    return ACOS()(ind);
}

inline Indicator ACOS(Indicator::value_t val) {
    return ACOS(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_ACOS_H_ */
