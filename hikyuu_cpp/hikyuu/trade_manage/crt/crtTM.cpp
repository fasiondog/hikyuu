/*
 * crtTM.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include "crtTM.h"

namespace hku {

TradeManagerPtr HKU_API crtTM(const Datetime& datetime, price_t initcash,
                              const TradeCostPtr& costfunc, const string& name) {
    return make_shared<TradeManager>(datetime, initcash, costfunc, name);
}

}  // namespace hku
