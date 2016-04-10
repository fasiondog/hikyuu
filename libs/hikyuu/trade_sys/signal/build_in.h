/*
 * build_in.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#ifndef SIGNAL_BUILD_IN_H_
#define SIGNAL_BUILD_IN_H_

#include "../../indicator/Operand.h"
#include "SignalBase.h"

namespace hku {

/**
 * 双曲线交叉信号指示器
 * @param fast 快线
 * @param slow 慢线
 * @return 信号指示器
 * @ingroup Signal
 */
SignalPtr HKU_API Cross_SG(const Operand& fast, const Operand& slow);

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

SignalPtr HKU_API Flex_SG(const Indicator& ind,
        double p = 2.0, const string& kpart = "CLOSE");

} /* namespace hku */


#endif /* SIGNAL_BUILD_IN_H_ */
