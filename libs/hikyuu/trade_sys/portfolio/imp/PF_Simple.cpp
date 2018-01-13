/*
 * PF_Simple.cpp
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#include "../crt/PF_Simple.h"

namespace hku {

PortfolioPtr HKU_API PF_Simple(
        const TradeManagerPtr& tm,
        const SystemPtr& sys,
        const SelectorPtr& st) {
    return make_shared<Portfolio>(tm, sys, st);
}

} /* namespace hku */


