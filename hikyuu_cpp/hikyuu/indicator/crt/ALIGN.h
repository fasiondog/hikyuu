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
Indicator HKU_API ALIGN(bool use_null = true);
Indicator HKU_API ALIGN(const DatetimeList&, bool use_null = true);
Indicator ALIGN(const Indicator& ind, const DatetimeList& ref, bool use_null = true);
Indicator ALIGN(const Indicator& ind, const Indicator& ref, bool use_null = true);
Indicator ALIGN(const Indicator& ind, const KData& ref, bool use_null = true);

inline Indicator ALIGN(const Indicator& ind, const DatetimeList& ref, bool use_null) {
    return ALIGN(ref, use_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const Indicator& ref, bool use_null) {
    return ALIGN(ref.getDatetimeList(), use_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const KData& ref, bool use_null) {
    return ALIGN(ref.getDatetimeList(), use_null)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ALIGN_H_ */
