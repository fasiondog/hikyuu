/*
 * crtZeroTC.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "../imp/ZeroTradeCost.h"
#include "crtZeroTC.h"

namespace hku {

TradeCostPtr HKU_API crtZeroTC() {
    return TradeCostPtr(new ZeroTradeCost());
}

} /* namespace */

