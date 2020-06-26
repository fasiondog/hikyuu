/*
 * PF_Simple.h
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_
#define TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_

#include "../Portfolio.h"
#include "../../selector/crt/SE_Fixed.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"

namespace hku {

PortfolioPtr HKU_API PF_Simple(const TMPtr& tm = TradeManagerPtr(), const SEPtr& se = SE_Fixed(),
                               const AFPtr& af = AF_EqualWeight());

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_IMP_PF_SIMPLE_H_ */
