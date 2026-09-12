/*
 * AF_FixedWeightList.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 *
 *  v5：AF 具备独立类层次（AllocateFundsBase），工厂返回 AFPtr。
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHTLIST_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHTLIST_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 固定比例资产分配
 * @details L1 按序逐一取 weights[i]，**不归一化**，
 *          等价 master FixedWeightListAllocateFunds 的 auto_adjust_weight=false
 * @param weights 指定的资产比例列表
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeightList(const vector<double>& weights);

}  // namespace hku

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHTLIST_H_ */
