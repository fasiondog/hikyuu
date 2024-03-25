/*
 * IStdev.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * 计算N周期内样本标准差
 * 参数： n: N日时间窗口
 * TODO      ma : 计算均值的函数原型
 *       link：均值参数联动标志，默认true
 *
 */
class IStdev : public hku::IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IStdev)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IStdev();
    virtual ~IStdev();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
