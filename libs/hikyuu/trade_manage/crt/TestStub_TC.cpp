/*
 * TestStub_TC.cpp
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */

#include "../imp/TradeCostStub.h"

namespace hku {

HKU_API TradeCostPtr TestStub_TC() {
    return TradeCostPtr(new TradeCostStub());
}

} /* namespace */


