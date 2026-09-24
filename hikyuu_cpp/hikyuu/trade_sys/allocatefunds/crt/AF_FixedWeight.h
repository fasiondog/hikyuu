/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Fixed proportion asset allocation, every selected asset accounts for a fixed proportion of
 * the total assets only
 * @param weight the given asset proportion (0, 1]
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeight(double weight = 0.1);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_ */
