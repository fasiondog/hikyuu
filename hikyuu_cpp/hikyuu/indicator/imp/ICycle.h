/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* 获取换手率，等于 VOL(k) / CAPITAL(k) */
class ICycle : public IndicatorImp {
    INDICATOR_IMP(ICycle)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICycle();
    explicit ICycle(const KData&);
    virtual ~ICycle();
    virtual void _checkParam(const string& name) const override;

private:
    void _initParams();
};

} /* namespace hku */
