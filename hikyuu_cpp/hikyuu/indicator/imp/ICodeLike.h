/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#pragma once
#ifndef INDICATOR_IMP_ICODELIKE_H_
#define INDICATOR_IMP_ICODELIKE_H_

#include "../Indicator.h"

namespace hku {

/* 以指标形式返回品种代码是否匹配指定模式 */
class ICodeLike : public IndicatorImp {
    INDICATOR_IMP(ICodeLike)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICodeLike();
    virtual ~ICodeLike() override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ICODELIKE_H_ */
