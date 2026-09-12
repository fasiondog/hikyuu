/*
 * AF_FixedWeight.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 *
 *  v5：AF 具备独立类层次（AllocateFundsBase），工厂返回 AFPtr。
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 固定比例资产分配，每个选中的资产都只占总资产固定的比例
 * @details L1 直接返回固定 weight，**不归一化**，
 *          等价 master FixedWeightAllocateFunds 的 auto_adjust_weight=false
 * @param weight 指定的资产比例 (0, 1]
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeight(double weight = 0.1);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_ */
