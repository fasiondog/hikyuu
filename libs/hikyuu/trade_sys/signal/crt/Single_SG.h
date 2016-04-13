/*
 * SINGLE_SG.h
 *
 *  Created on: 2015年2月22日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_
#define TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Single_SG
 * @param ind
 * @param filter_n
 * @param filter_p
 * @param kpart
 * @return
 * @ingroup Signal
 */
SignalPtr HKU_API Single_SG(const Operand& ind,
        int filter_n = 20, double filter_p = 0.1,
        const string& kpart = "CLOSE");

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_ */
