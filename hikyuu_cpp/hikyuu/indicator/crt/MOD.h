/*
 * MOD.h
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_MOD_H_
#define INDICATOR_IMP_MOD_H_

#include "CVAL.h"

namespace hku {

/**
 * Modulo after rounding
 * @details
 * <pre>
 * This function exists only for the TDX compatibility. In fact, the modulo of the indicators can be
 * done directly with the % operator
 * </pre>
 * @param ind1 indicator 1, it is rounded
 * @param ind2 indicator 2, it is rounded
 * @ingroup Indicator
 */

inline Indicator MOD(const Indicator& ind1, const Indicator& ind2) {
    return (ind1 % ind2);
}

inline Indicator MOD(const Indicator& ind1, Indicator::value_t ind2) {
    return ind1 % CVAL(ind1, ind2);
}

inline Indicator MOD(Indicator::value_t ind1, const Indicator& ind2) {
    return CVAL(ind2, ind1) % ind2;
}

inline Indicator MOD(Indicator::value_t ind1, Indicator::value_t ind2) {
    return CVAL(ind1) % CVAL(ind2);
}

}  // namespace hku

#endif /* INDICATOR_IMP_MOD_H_ */
