/*
 * CROSS_SG.h
 *
 *  Created on: 2015-2-20
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_CROSS_H_
#define TRADE_SYS_SIGNAL_CRT_SG_CROSS_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Two line crossing indicator: it buys when the fast line crosses the slow line upward from below,
 * and sells when the fast line crosses the slow line downward from above.
 * @param fast fast line
 * @param slow slow line
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Cross(const Indicator& fast, const Indicator& slow);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_CROSS_H_ */
