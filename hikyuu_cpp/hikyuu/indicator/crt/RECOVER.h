/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240317 added by fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API RECOVER_FORWARD();
Indicator HKU_API RECOVER_BACKWARD();
Indicator HKU_API RECOVER_EQUAL_FORWARD();
Indicator HKU_API RECOVER_EQUAL_BACKWARD();

Indicator HKU_API RECOVER_FORWARD(const Indicator&);
Indicator HKU_API RECOVER_BACKWARD(const Indicator&);
Indicator HKU_API RECOVER_EQUAL_FORWARD(const Indicator&);
Indicator HKU_API RECOVER_EQUAL_BACKWARD(const Indicator&);

inline Indicator RECOVER_FORWARD(const KData& kdata) {
    return RECOVER_FORWARD(kdata.close());
}

inline Indicator RECOVER_BACKWARD(const KData& kdata) {
    return RECOVER_BACKWARD(kdata.close());
}

inline Indicator RECOVER_EQUAL_FORWARD(const KData& kdata) {
    return RECOVER_EQUAL_FORWARD(kdata.close());
}

inline Indicator RECOVER_EQUAL_BACKWARD(const KData& kdata) {
    return RECOVER_EQUAL_BACKWARD(kdata.close());
}

}  // namespace hku