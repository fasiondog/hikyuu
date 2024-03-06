/*
 * LN.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LN_H_
#define INDICATOR_CRT_LN_H_

#include "CVAL.h"

namespace hku {

/**
 * 求自然对数
 * 用法：LN(X)以e为底的对数
 * 例如：LN(CLOSE)求收盘价的对数
 * @ingroup Indicator
 */
Indicator HKU_API LN();
Indicator LN(Indicator::value_t);
Indicator LN(const Indicator& ind);

inline Indicator LN(const Indicator& ind) {
    return LN()(ind);
}

inline Indicator LN(Indicator::value_t val) {
    return LN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_LN_H_ */
