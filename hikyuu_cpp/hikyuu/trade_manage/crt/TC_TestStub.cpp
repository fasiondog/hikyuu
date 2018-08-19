/*
 * TC_TestStub.cpp
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */

#include "../imp/TradeCostStub.h"

namespace hku {

HKU_API TradeCostPtr TC_TestStub() {
    return TradeCostPtr(new TradeCostStub());
}

} /* namespace */


