/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IINBLOCK_H_
#define INDICATOR_IMP_IINBLOCK_H_

#include "../Indicator.h"

namespace hku {

/* 已指标形式返回是否在指定板块中 */
class IInBlock : public IndicatorImp {
    INDICATOR_IMP(IInBlock)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IInBlock();
    explicit IInBlock(const KData& kdata, const string& category, const string& name);
    virtual ~IInBlock();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_IINBLOCK_H_ */
