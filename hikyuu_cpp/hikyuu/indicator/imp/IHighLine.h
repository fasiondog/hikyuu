/*
 * IHighLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016年4月1日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IHIGHLINE_H_
#define INDICATOR_IMP_IHIGHLINE_H_

#include "../Indicator.h"

namespace hku {

/*
 * N日内最高价，一般使用最高价数据作为输入
 * 参数： n: N日时间窗口
 */
class IHighLine : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IHighLine)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IHighLine();
    virtual ~IHighLine();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_IHIGHLINE_H_ */
