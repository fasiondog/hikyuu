/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBLOCKSETNUM_H_
#define INDICATOR_IMP_IBLOCKSETNUM_H_

#include "../Indicator.h"

namespace hku {

class IBlockSetNum : public IndicatorImp {
    INDICATOR_IMP(IBlockSetNum)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBlockSetNum();
    virtual ~IBlockSetNum();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku

#endif /* INDICATOR_IMP_IBLOCKSETNUM_H_ */