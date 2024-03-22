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
Indicator HKU_API ALIGN(bool fill_null = true);
Indicator HKU_API ALIGN(const DatetimeList&, bool fill_null = true);
Indicator ALIGN(const Indicator& ind, const DatetimeList& ref, bool fill_null = true);
Indicator ALIGN(const Indicator& ind, const Indicator& ref, bool fill_null = true);
Indicator ALIGN(const Indicator& ind, const KData& ref, bool fill_null = true);

inline Indicator ALIGN(const Indicator& ind, const DatetimeList& ref, bool fill_null) {
    return ALIGN(ref, fill_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const Indicator& ref, bool fill_null) {
    return ALIGN(ref.getDatetimeList(), fill_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const KData& ref, bool fill_null) {
    return ALIGN(ref.getDatetimeList(), fill_null)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ALIGN_H_ */
