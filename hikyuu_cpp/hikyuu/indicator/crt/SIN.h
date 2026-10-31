/*
 * SIN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SIN_H_
#define INDICATOR_CRT_SIN_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Sine value
 * @ingroup Indicator
 */
Indicator HKU_API SIN();

inline Indicator SIN(const Indicator& ind) {
    return SIN()(ind);
}

inline Indicator SIN(Indicator::value_t val) {
    return SIN(CVAL(val));
}

inline void registerPart_SIN() {
    registerIndicatorPart("core.ind.sin", [] { return SIN(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_SIN_H_ */
