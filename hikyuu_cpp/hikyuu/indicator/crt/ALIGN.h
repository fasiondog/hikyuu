/*
 * ALIGN.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-21
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ALIGN_H_
#define INDICATOR_CRT_ALIGN_H_

#include "../Indicator.h"

namespace hku {

/**
 * 按指定日期对齐
 * @ingroup Indicator
 */
Indicator HKU_API ALIGN(const DatetimeList&);
Indicator ALIGN(const Indicator& ind, const DatetimeList& ref);
Indicator ALIGN(const Indicator& ind, const Indicator& ref);
Indicator ALIGN(const Indicator& ind, const KData& ref);

inline Indicator ALIGN(const Indicator& ind, const DatetimeList& ref) {
    return ALIGN(ref)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const Indicator& ref) {
    return ALIGN(ref.getDatetimeList())(ind);
}

inline Indicator ALIGN(const Indicator& ind, const KData& ref) {
    return ALIGN(ref.getDatetimeList())(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ALIGN_H_ */
