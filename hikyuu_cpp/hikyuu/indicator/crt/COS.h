/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_COS_H_
#define INDICATOR_CRT_COS_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Cosine value
 * @ingroup Indicator
 */
Indicator HKU_API COS();

inline Indicator COS(const Indicator& ind) {
    return COS()(ind);
}

inline Indicator COS(Indicator::value_t val) {
    return COS(CVAL(val));
}

inline void registerPart_COS() {
    registerIndicatorPart("core.ind.cos", [] { return COS(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_COS_H_ */
