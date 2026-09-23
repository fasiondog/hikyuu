/*
 * AF_EqualWeight.h
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 *
 *  v5: AF has an independent class hierarchy (AllocateFundsBase), the factory returns AFPtr.
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Equal weight asset allocation, it allocates the selected assets in equal proportions
 * @details L1 default equal weight 1/N (equivalent to the normalized equal-proportion semantics of
 *          master EqualWeightAllocateFunds)
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_EqualWeight();

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_ */
