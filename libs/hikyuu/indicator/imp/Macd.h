/*
 * Macd.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#ifndef MACD_H_
#define MACD_H_

#include "../Indicator.h"

namespace hku {

/*
 * MACD平滑异同移动平均线
 * 参数： n1: 短期EMA时间窗
 *         n2: 长期EMA时间窗
 *         n3: （短期EMA-长期EMA）EMA平滑时间窗
 * 返回：1)MACD BAR： MACD直柱，即MACD快线－MACD慢线
 *      2)DIFF: 快线,即（短期EMA-长期EMA）
 *      3)DEA: 慢线，即快线的n3周期EMA平滑
 */
class Macd: public IndicatorImp {
    INDICATOR_IMP(Macd)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Macd();
    virtual ~Macd();
};

} /* namespace hku */
#endif /* MACD_H_ */
