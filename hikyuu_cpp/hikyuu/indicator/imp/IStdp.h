/*
 * IStdp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISTDP_H_
#define INDICATOR_IMP_ISTDP_H_

#include "../Indicator.h"

namespace hku {

/*
 * 计算N周期内总体标准差
 * 参数： n: N日时间窗口
 */
class IStdp : public hku::IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IStdp)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IStdp();
    virtual ~IStdp();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISTDP_H_ */
