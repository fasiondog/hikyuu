/*
 * AF_MultiFactor.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 *
 *  AF has an independent class hierarchy (AllocateFundsBase), the factory returns AFPtr.
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * Create an asset allocation algorithm instance with the MultiFactor score weights, i.e. the scores
 * returned by SE are used as the weights directly.
 * @details L1 uses SubSystemContext::score as the weight. It depends on MultiSystem back-filling
 * the SE score before L1.
 * @param auto_adjust_weight when true (the default) the scores are treated as the mutual ratios and
 *        normalized to sum 1 (the 2.8.3 semantics); when false the score itself is the final weight
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_MultiFactor(bool auto_adjust_weight = true);

}  // namespace hku

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_ */
