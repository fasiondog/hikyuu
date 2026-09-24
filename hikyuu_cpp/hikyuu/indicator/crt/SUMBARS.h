/*
 * SUMBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SUMBARS_H_
#define INDICATOR_CRT_SUMBARS_H_

#include "../Indicator.h"

namespace hku {

/**
 * Accumulate to the given number of periods; the number of periods from the accumulation forward to
 * the given value until now
 * @details
 * <pre>
 * Usage: SUMBARS(X,A): accumulate X forward until it is greater than or equal to A, and return the
 * number of periods of this interval
 * For example: SUMBARS(VOL,CAPITAL) gives the number of periods from the full turnover until now
 * </pre>
 * @note discard semantics (there is a difference between the scalar parameter and the sequence
 *       parameter):
 *  - Scalar parameter `SUMBARS(ind, double)`: if a position is still < a after accumulating to the
 *    leftmost end of the sequence, the whole segment is marked as discard
 *    (a static global optimization, because when the scalar a is unreachable monotonically the
 *    earlier positions are even more unreachable).
 *  - Sequence parameter `SUMBARS(ind, IndParam)`: NaN is written to every unreachable position, but
 *    discard is not advanced
 *    (the dynamic a sequence is not monotonic; a[i] being unreachable does not mean a[i+1] is
 *    unreachable, and advancing would wipe out the later calculable positions).
 *  That is, the dynamic path does not guarantee "all the values after discard are valid". The
 *  downstream should handle it with `std::isnan`,
 *  and should not assume "everything after discard is valid".
 * @ingroup Indicator
 */
Indicator HKU_API SUMBARS(double a);
Indicator HKU_API SUMBARS(const IndParam& a);

inline Indicator SUMBARS(const Indicator& ind, double a) {
    return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const IndParam& a) {
    return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const Indicator& a) {
    return SUMBARS(IndParam(a))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SUMBARS_H_ */
