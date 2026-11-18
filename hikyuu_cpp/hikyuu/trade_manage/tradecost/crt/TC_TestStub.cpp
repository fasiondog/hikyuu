/*
 * TC_TestStub.cpp
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */

#include "../imp/TradeCostStub.h"

namespace hku {

HKU_API TradeCostPtr TC_TestStub() {
    return make_shared<TradeCostStub>();
}

}  // namespace hku
