/*
 * ALIGN.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-5-21
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ALIGN_H_
#define INDICATOR_CRT_ALIGN_H_

#include "../Indicator.h"

namespace hku {

/**
 * 按指定日期对齐
 * @ingroup Indicator
 */
Indicator HKU_API ALIGN();
Indicator HKU_API ALIGN(const DatetimeList&);

Indicator ALIGN(const Indicator& ind) {
    return ALIGN()(ind);
}

Indicator ALIGN(const Indicator& ind, const DatetimeList& ref) {
    return ALIGN(ref)(ind);
}

}

#endif /* INDICATOR_CRT_ALIGN_H_ */
