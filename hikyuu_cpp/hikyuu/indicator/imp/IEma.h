/*
 * IEma.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * 指数移动平均线(Exponential Moving Average)
 * 参数： n: 计算均值的周期窗口，必须为大于0的整数
 * 抛弃数 = 0
 */
class IEma : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IEma)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IEma();
    virtual ~IEma();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
