/*
 * POS.h
 *
 *  Created on: 2015年3月17日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_POS_H_
#define INDICATOR_CRT_POS_H_

#include "../../trade_sys/signal/SignalBase.h"
#include "../../Block.h"

namespace hku {

Indicator HKU_API POS(const Block& block, KQuery query, SignalPtr ind);

} /* namespace hku */

#endif /* INDICATOR_CRT_POS_H_ */
