#pragma once
#include <hikyuu/indicator/IndicatorImp.h>

namespace hku {

class IAdx2 : public IndicatorImp {
    INDICATOR_IMP(IAdx2)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION
public:
    IAdx2();
    virtual ~IAdx2() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku