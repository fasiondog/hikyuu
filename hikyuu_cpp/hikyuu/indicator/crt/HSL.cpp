/*
 * HSL.cpp
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */


#include "KDATA.h"
#include "LIUTONGPAN.h"
#include "HSL.h"

namespace hku {

Indicator HKU_API HSL(const KData& k) {
    return VOL(k) / LIUTONGPAN(k);
}

} /* namespace */

