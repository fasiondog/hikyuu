/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 固定比例资产分配，每个选中的资产都只占总资产固定的比例
 * @param weight 指定的资产比例 (0, 1]
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeight(double weight = 0.1);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDWEIGHT_H_ */
