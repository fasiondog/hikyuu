/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2025-12-9
 *      Author: stone
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Fixed amount allocation, the trade of every selected asset cannot exceed this amount
 * @param amount the given fixed trade amount
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedAmount(double amount = 20000.0);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_ */
