/*
 * tm_build_in.cpp
 *
 *  Created on: 2016年4月10日
 *      Author: Administrator
 */

#include "build_in.h"
#include "imp/FixedATradeCost.h"
#include "imp/ZeroTradeCost.h"
#include "imp/TradeCostStub.h"

namespace hku {

TradeManagerPtr HKU_API crtTM(const Datetime& datetime, price_t initcash,
        const TradeCostPtr& costfunc, const string& name) {
    return TradeManagerPtr(new TradeManager(datetime, initcash, costfunc, name));
}


TradeCostPtr HKU_API crtZeroTC() {
    return TradeCostPtr(new ZeroTradeCost());
}

TradeCostPtr HKU_API crtFixedATC(
        price_t commission,
        price_t lowestCommission,
        price_t stamptax,
        price_t transferfee,
        price_t lowestTransferfee) {
    return TradeCostPtr(new FixedATradeCost(commission, lowestCommission,
            stamptax, transferfee, lowestTransferfee));
}

TradeCostPtr HKU_API TestStub_TC() {
    return TradeCostPtr(new TradeCostStub());
}

} /* namespace hku */


