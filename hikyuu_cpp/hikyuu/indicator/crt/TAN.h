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
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Tangent value
 * @ingroup Indicator
 */
Indicator HKU_API TAN();

inline Indicator TAN(const Indicator& ind) {
    return TAN()(ind);
}

inline Indicator TAN(Indicator::value_t val) {
    return TAN(CVAL(val));
}

inline void registerPart_TAN() {
    registerIndicatorPart("core.ind.tan", [] { return TAN(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_TAN_H_ */
