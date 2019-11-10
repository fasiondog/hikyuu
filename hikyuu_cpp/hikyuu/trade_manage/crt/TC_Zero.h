/*
 * TC_Zero.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef CRTZEROTC_H_
#define CRTZEROTC_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * 创建零成本算法实例
 * @see ZeroTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_Zero();

}  // namespace hku

#endif /* CRTZEROTC_H_ */
