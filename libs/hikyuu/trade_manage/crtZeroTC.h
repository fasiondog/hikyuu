/*
 * crtZeroTC.h
 *
 *  Created on: 2016年4月11日
 *      Author: Administrator
 */

#ifndef TRADE_MANAGE_CRTZEROTC_H_
#define TRADE_MANAGE_CRTZEROTC_H_

#include "TradeCostBase.h"

namespace hku {

/**
 * 创建零成本算法实例
 * @see ZeroTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API crtZeroTC();

} /* namespace hku */

#endif /* TRADE_MANAGE_CRTZEROTC_H_ */
