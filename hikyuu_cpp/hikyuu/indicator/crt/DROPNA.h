/*
 * DROPNA.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DROPNA_H_
#define INDICATOR_CRT_DROPNA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 删除nan值
 * @ingroup Indicator
 */
Indicator HKU_API DROPNA();
Indicator DROPNA(const Indicator& ind);

inline Indicator DROPNA(const Indicator& ind) {
    return DROPNA()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_DROPNA_H_ */
