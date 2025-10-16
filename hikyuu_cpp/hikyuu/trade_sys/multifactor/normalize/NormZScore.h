/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../NormalizeBase.h"

namespace hku {

class NormZScore : public NormalizeBase {
    NORMALIZE_IMP(NormZScore)
    NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormZScore();
    NormZScore(bool outExtreme, double nsigma, bool recursive);
    virtual ~NormZScore() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku