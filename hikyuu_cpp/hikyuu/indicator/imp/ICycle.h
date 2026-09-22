/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* Get the turnover rate, it equals VOL(k) / CAPITAL(k) */
class ICycle : public IndicatorImp {
    INDICATOR_IMP(ICycle)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICycle();
    virtual ~ICycle() override;
    virtual void _checkParam(const string& name) const override;

private:
    void _initParams();
};

} /* namespace hku */
