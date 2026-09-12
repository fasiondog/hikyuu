/*
 * FixedAmountAllocateFunds.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  AF_FixedAmount 的语义载体：L1 等权 + L2 固定金额换算（参数 fixed-amount）。
 */

#pragma once
#ifndef FIXEDAMOUNTALLOCATEFUNDS_H_
#define FIXEDAMOUNTALLOCATEFUNDS_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 固定金额资产分配，每个选中的资产都不能超过此金额交易。
 * @details L1 使用基类默认等权；L2 读取参数 fixed-amount：
 *          - 模式 A：每条建议的目标市值取固定金额；
 *          - 模式 B：每个子系统的固定额度（写入 contexts[i].quota）。
 * @note 与 master FixedAmountFunds 存在边界差异：master 断言 amount>500 且剩余现金 <0.6×amount
 *       时停止，新体系为 >=0 且按净额调仓。
 * @param fixed-amount 指定的固定交易金额
 * @ingroup AllocateFunds
 */
class FixedAmountAllocateFunds : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(FixedAmountAllocateFunds)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedAmountAllocateFunds();
    virtual ~FixedAmountAllocateFunds();
};

}  // namespace hku

#endif /* FIXEDAMOUNTALLOCATEFUNDS_H_ */
