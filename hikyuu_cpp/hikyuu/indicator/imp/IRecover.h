/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240317 added by fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IRecover : public IndicatorImp {
    INDICATOR_IMP(IRecover)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRecover();
    explicit IRecover(int recoverType);
    IRecover(const KData&, int recoverType);
    virtual ~IRecover();

    virtual void _checkParam(const string& name) const override;

    static void checkInputIndicator(const Indicator& ind);
    virtual bool supportIncrementCalculate() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku