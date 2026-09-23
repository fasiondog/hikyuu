/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LASTBAR_H_
#define INDICATOR_CRT_LASTBAR_H_

#include "../Indicator.h"

namespace hku {

/**
 * Judge whether the current data is the last one; it returns 1 if it is the last one, otherwise 0.
 * @ingroup Indicator
 */
Indicator HKU_API ISLASTBAR();
Indicator HKU_API ISLASTBAR(const KData& kdata);

inline Indicator ISLASTBAR(const Indicator& ind) {
    return ISLASTBAR()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LASTBAR_H_ */
