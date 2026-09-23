/*
 * AF_FixedAmount.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2025-12-9
 *      Author: stone
 *
 *  v5: AF has an independent class hierarchy (AllocateFundsBase), the factory returns AFPtr.
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Fixed amount allocation, the trade of every selected asset cannot exceed this amount
 * @details L1 equal weight + L2 reads the parameter fixed-amount (mode A: a fixed amount per
 *          instrument / mode B: a fixed quota per sub-system). There are boundary differences with
 *          master FixedAmountFunds: master asserts amount>500 and stops when the remaining cash is
 *          less than 0.6×amount, while the new system uses >=0 and rebalances by the net amount.
 * @param amount the given fixed trade amount
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedAmount(double amount = 20000.0);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_ */
