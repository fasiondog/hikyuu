/*
 * AF_MultiFactor.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 *
 *  v5：AF 具备独立类层次（AllocateFundsBase），工厂返回 AFPtr。
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 创建 MultiFactor 评分权重的资产分配算法实例，即直接以SE返回的评分作为权重。
 * @details L1 以 SubSystemContext::score 为权重。
 *          依赖 MultiSystem 在 L1 前回填 SE 得分（design.md §5.3 / O2）。
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_MultiFactor();

}  // namespace hku

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_MULTIFACTOR_H_ */
