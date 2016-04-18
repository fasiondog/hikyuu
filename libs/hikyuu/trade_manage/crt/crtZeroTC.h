/*
 * crtZeroTC.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#ifndef CRTZEROTC_H_
#define CRTZEROTC_H_

#include "../TradeCostBase.h"

#define TC_Zero crtZeroTC

namespace hku {

/**
 * 创建零成本算法实例
 * @see ZeroTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API crtZeroTC();

} /* namespace */

#endif /* CRTZEROTC_H_ */
