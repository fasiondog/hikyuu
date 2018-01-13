/*
 * PF_Simple.h
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_
#define TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_

#include "../Portfolio.h"
#include "../../system/crt/SYS_Simple.h"
#include "../../selector/crt/SE_Fixed.h"

namespace hku {

PortfolioPtr HKU_API PF_Simple(
        const TradeManagerPtr& tm = TradeManagerPtr(),
        const SystemPtr& sys = SYS_Simple(),
        const SelectorPtr& st = SE_Fixed());

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_ */
