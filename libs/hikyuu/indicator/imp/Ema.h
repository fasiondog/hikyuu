/*
 * Ema.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#ifndef EMA_H_
#define EMA_H_

#include "../Indicator.h"

namespace hku {

/*
 * 指数移动平均线(Exponential Moving Average)
 * 参数： n: 计算均值的周期窗口，必须为大于0的整数
 * 抛弃数 = 0
 */
class Ema: public IndicatorImp {
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Ema(int n = 22);
    Ema(const Indicator&, int n = 22);
    virtual ~Ema();

    virtual string name() const;
    virtual IndicatorImpPtr operator()(const Indicator& ind);
};

} /* namespace hku */
#endif /* EMA_H_ */
