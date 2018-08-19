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
 * 单线拐点信号指示器
 * @details
 * <pre>
 * 使用《精明交易者》中给出的曲线拐点算法判断曲线趋势，公式如下：
 *
 *     filter = percentage * STDEV((AMA-AMA[1], N)
 *     Buy  When AMA - AMA[1] > filter
 *     or Buy When AMA - AMA[2] > filter
 *     or Buy When AMA - AMA[3] > filter
 * </pre>
 * @param ind
 * @param filter_n N日周期
 * @param filter_p 过滤器百分比
 * @param kpart KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
 * @return
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Single(const Operand& ind,
        int filter_n = 20, double filter_p = 0.1,
        const string& kpart = "CLOSE");

/**
 * 单线拐点信号指示器2
 * @details
 * <pre>
 * 使用《精明交易者》中给出的曲线拐点算法判断曲线趋势，公式如下：
 *
 *     filter = percentage * STDEV((AMA-AMA[1], N)
 *     Buy  When AMA - @lowest(AMA,n) > filter
 *     Sell When @highest(AMA, n) - AMA > filter
 * </pre>
 * @param ind
 * @param filter_n N日周期
 * @param filter_p 过滤器百分比
 * @param kpart KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
 * @return
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Single2(const Operand& ind,
        int filter_n = 20, double filter_p = 0.1,
        const string& kpart = "CLOSE");

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_ */
