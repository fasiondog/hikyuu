/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API RESULT(int result_ix);

inline Indicator HKU_API RESULT(const Indicator& ind, int result_ix) {
    return RESULT(result_ix)(ind);
}

}