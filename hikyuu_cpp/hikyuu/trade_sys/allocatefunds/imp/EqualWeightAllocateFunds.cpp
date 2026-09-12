/*
 * EqualWeightAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 */

#include "EqualWeightAllocateFunds.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::EqualWeightAllocateFunds)
#endif

namespace hku {

EqualWeightAllocateFunds::EqualWeightAllocateFunds() : AllocateFundsBase("AF_EqualWeight") {}

EqualWeightAllocateFunds::~EqualWeightAllocateFunds() {}

}  // namespace hku
