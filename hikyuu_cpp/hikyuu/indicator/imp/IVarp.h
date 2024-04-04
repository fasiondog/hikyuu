/*
 * IVarp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IVARP_H_
#define INDICATOR_IMP_IVARP_H_

#include "../Indicator.h"

namespace hku {

/*
 * 估算总体样本方差
 */
class IVarp : public hku::IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IVarp)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IVarp();
    virtual ~IVarp();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IVARP_H_ */
