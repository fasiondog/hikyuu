/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#pragma once
#ifndef INDICATOR_IMP_INAMELIKE_H_
#define INDICATOR_IMP_INAMELIKE_H_

#include "../Indicator.h"

namespace hku {

/* 以指标形式返回品种名称是否匹配指定模式 */
class INameLike : public IndicatorImp {
    INDICATOR_IMP(INameLike)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    INameLike();
    virtual ~INameLike() override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_INAMELIKE_H_ */
