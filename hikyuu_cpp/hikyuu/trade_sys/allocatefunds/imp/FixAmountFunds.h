#pragma once
#include "../AllocateFundsBase.h"

namespace hku {

class FixedAmountFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(FixedAmountFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedAmountFunds();
    virtual ~FixedAmountFunds();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */


