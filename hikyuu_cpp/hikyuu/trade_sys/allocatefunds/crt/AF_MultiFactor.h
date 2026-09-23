/*
 * AF_MultiFactor.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 *
 *  v5: AF has an independent class hierarchy (AllocateFundsBase), the factory returns AFPtr.
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * Create an asset allocation algorithm instance with the MultiFactor score weights, i.e. the scores
 * returned by SE are used as the weights directly.
 * @details L1 uses SubSystemContext::score as the weight. It depends on MultiSystem back-filling the
 *          SE score before L1 (design.md §5.3 / O2).
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_MultiFactor();

}  // namespace hku

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_ */
