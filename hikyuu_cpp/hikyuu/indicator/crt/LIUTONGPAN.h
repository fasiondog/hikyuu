/*
 * LIUTONGPANG.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LIUTONGPAN_H_
#define INDICATOR_CRT_LIUTONGPAN_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

#define CAPITAL LIUTONGPAN

namespace hku {

/** Outstanding shares, in units of 10 thousand shares */
Indicator HKU_API LIUTONGPAN();
Indicator HKU_API LIUTONGPAN(const KData&);

inline void registerPart_LIUTONGPAN() {
    registerIndicatorPart("core.ind.liutongpan", [] { return LIUTONGPAN(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_LIUTONGPAN_H_ */
