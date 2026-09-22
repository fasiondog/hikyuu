/*
 * CROSS_SG.h
 *
 *  Created on: 2015-2-20
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_CROSSGOLD_H_
#define TRADE_SYS_SIGNAL_CRT_SG_CROSSGOLD_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Golden cross indicator: it is a golden cross and a buy when the fast line crosses the slow line
 * upward from below and both the fast line and the slow line are directed upward;
 * it is a death cross and a sell when the fast line crosses the slow line downward from above and
 * both the fast line and the slow line are directed downward.
 * @param fast fast line
 * @param slow slow line
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HKU_API SG_CrossGold(const Indicator& fast, const Indicator& slow);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_CROSSGOLD_H_ */
