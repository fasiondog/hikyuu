/*
 * crtFixedATC.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "../imp/FixedATradeCost.h"

namespace hku {

TradeCostPtr HKU_API crtFixedATC(
        price_t commission,
        price_t lowestCommission,
        price_t stamptax,
        price_t transferfee,
        price_t lowestTransferfee) {
    return TradeCostPtr(new FixedATradeCost(commission, lowestCommission,
            stamptax, transferfee, lowestTransferfee));
}

} /* namespace */


