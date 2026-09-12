/*
 * AF_FixedAmount.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2025年12月9日
 *      Author: stone
 *
 *  v5：AF 具备独立类层次（AllocateFundsBase），工厂返回 AFPtr。
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 固定金额分配，每个选中的资产都不能超过此金额交易
 * @details L1 等权 + L2 读取参数 fixed-amount（模式 A 每标的固定金额 /
 *          模式 B 每子系统固定额度）。与 master FixedAmountFunds 存在边界差异：
 *          master 断言 amount>500 且剩余现金 <0.6×amount 时停止，新体系为 >=0 且按净额调仓。
 * @param amount 指定的固定交易金额
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedAmount(double amount = 20000.0);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_FIXEDAMOUNT_H_ */
