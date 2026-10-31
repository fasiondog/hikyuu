/*
 * LOG.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LOG_H_
#define INDICATOR_CRT_LOG_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Logarithm with the base 10
 * Usage: LOG(X) gets the logarithm of X
 * @note Undefined domain (X <= 0) returns null
 * @ingroup Indicator
 */
Indicator HKU_API LOG();

inline Indicator LOG(const Indicator& ind) {
    return LOG()(ind);
}

inline Indicator LOG(Indicator::value_t val) {
    return LOG(CVAL(val));
}

inline void registerPart_LOG() {
    registerIndicatorPart("core.ind.log", [] { return LOG(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_LOG_H_ */
