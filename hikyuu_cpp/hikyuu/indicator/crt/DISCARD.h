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

#include "../Indicator.h"

namespace hku {

/** Set the discard value in the way of an indicator formula */
Indicator HKU_API DISCARD(int discard);

inline Indicator DISCARD(const Indicator& ind, int discard) {
    return DISCARD(discard)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_DISCARD_H_ */
