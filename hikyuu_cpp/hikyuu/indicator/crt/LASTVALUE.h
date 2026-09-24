/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LASTVALUE_H_
#define INDICATOR_CRT_LASTVALUE_H_

#include "../Indicator.h"

namespace hku {

/**
 * Take the last value of the input indicator as a constant, i.e. all the values in the result are
 * the last value of the input indicator; use it with caution
 * @note It is equivalent to the TDX CONST indicator; it is named LASTVALUE because of the name
 *       conflict of CONST under Windows
 * @ingroup Indicator
 */
Indicator HKU_API LASTVALUE(bool ignore_discard = false);

inline Indicator LASTVALUE(const Indicator& ind, bool ignore_discard = false) {
    return LASTVALUE(ignore_discard)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LASTVALUE_H_ */
