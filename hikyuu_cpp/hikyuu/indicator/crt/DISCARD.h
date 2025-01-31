/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DISCARD_H_
#define INDICATOR_CRT_DISCARD_H_

#include "CVAL.h"

namespace hku {

/** 以指标公式的方式设置抛弃值 */
Indicator HKU_API DISCARD(int discard);

inline Indicator DISCARD(const Indicator& ind, int discard) {
    return DISCARD(discard)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_DISCARD_H_ */
