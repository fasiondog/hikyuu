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
 * Remove the nan values
 * @note A row is removed once any of its result sets is nan, so every result set keeps the same
 * rows and the same length
 * @ingroup Indicator
 */
Indicator HKU_API DROPNA();

inline Indicator DROPNA(const Indicator& ind) {
    return DROPNA()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_DROPNA_H_ */
