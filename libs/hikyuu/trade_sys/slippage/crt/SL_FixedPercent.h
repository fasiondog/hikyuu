/*
 * SL_FixedPercent.h
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_SLIPPAGE_CRT_SL_FIXEDPERCENT_H_
#define TRADE_SYS_SLIPPAGE_CRT_SL_FIXEDPERCENT_H_

#include "../SlippageBase.h"

namespace hku {

SlippagePtr HKU_API SL_FixedPercent(double p = 0.001);

} /* namespace hku */


#endif /* TRADE_SYS_SLIPPAGE_CRT_SL_FIXEDPERCENT_H_ */
