/*
 * LIUTONGPANG.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LIUTONGPAN_H_
#define INDICATOR_CRT_LIUTONGPAN_H_

#include "../Indicator.h"

#define CAPITAL LIUTONGPAN

namespace hku {

/** Outstanding shares, in units of 10 thousand shares */
Indicator HKU_API LIUTONGPAN();
Indicator HKU_API LIUTONGPAN(const KData&);

}  // namespace hku

#endif /* INDICATOR_CRT_LIUTONGPAN_H_ */
