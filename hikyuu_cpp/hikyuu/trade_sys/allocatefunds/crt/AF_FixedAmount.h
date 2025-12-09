/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2025年12月9日
 *      Author: stone
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 固定金额分配，每个选中的资产都不能超过此金额交易
 * @param amount 指定的固定交易金额
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedAmount(double amount = 20000.0);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_ */
