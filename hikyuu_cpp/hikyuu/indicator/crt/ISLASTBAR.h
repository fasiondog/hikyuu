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
 * 判断当前数据是否为最后一个数据，若为最后一个数据，则返回1，否则返回0.
 * @ingroup Indicator
 */
Indicator HKU_API ISLASTBAR();
Indicator HKU_API ISLASTBAR(const KData& kdata);
Indicator ISLASTBAR(const Indicator& ind);

inline Indicator ISLASTBAR(const Indicator& ind) {
    return ISLASTBAR()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LASTBAR_H_ */
