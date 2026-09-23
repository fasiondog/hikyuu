/*
 * AF_FixedWeightList.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 *
 *  v5: AF has an independent class hierarchy (AllocateFundsBase), the factory returns AFPtr.
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHTLIST_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHTLIST_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Fixed proportion asset allocation
 * @details L1 takes weights[i] one by one in order, **without normalization**, equivalent to master
 *          FixedWeightListAllocateFunds with auto_adjust_weight=false
 * @param weights the given asset proportion list
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeightList(const vector<double>& weights);

}  // namespace hku

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHTLIST_H_ */
