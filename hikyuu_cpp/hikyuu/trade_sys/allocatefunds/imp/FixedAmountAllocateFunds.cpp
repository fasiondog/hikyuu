/*
 * FixedAmountAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 */

#include "FixedAmountAllocateFunds.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedAmountAllocateFunds)
#endif

namespace hku {

FixedAmountAllocateFunds::FixedAmountAllocateFunds() : AllocateFundsBase("AF_FixedAmount") {
    setParam<double>("fixed-amount", 20000.0);
}

FixedAmountAllocateFunds::~FixedAmountAllocateFunds() {}

}  // namespace hku
