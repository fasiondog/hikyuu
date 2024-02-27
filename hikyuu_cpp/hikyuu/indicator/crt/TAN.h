/*
 * TAN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TAN_H_
#define INDICATOR_CRT_TAN_H_

#include "CVAL.h"

namespace hku {

/**
 * 正切值
 * @ingroup Indicator
 */
Indicator HKU_API TAN();
Indicator TAN(Indicator::value_t);
Indicator TAN(const Indicator& ind);

inline Indicator TAN(const Indicator& ind) {
    return TAN()(ind);
}

inline Indicator TAN(Indicator::value_t val) {
    return TAN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_TAN_H_ */
