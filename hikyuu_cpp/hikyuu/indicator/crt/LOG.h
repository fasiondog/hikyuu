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

namespace hku {

/**
 * Logarithm with the base 10
 * Usage: LOG(X) gets the logarithm of X
 * @ingroup Indicator
 */
Indicator HKU_API LOG();

inline Indicator LOG(const Indicator& ind) {
    return LOG()(ind);
}

inline Indicator LOG(Indicator::value_t val) {
    return LOG(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_LOG_H_ */
