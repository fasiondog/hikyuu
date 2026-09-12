/*
 * AF_EqualWeight.h
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 *
 *  v5：AF 具备独立类层次（AllocateFundsBase），工厂返回 AFPtr。
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 等权重资产分配，对选中的资产进行等比例分配
 * @details L1 默认等权 1/N（等价 master EqualWeightAllocateFunds 归一化后的等比例语义）
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_EqualWeight();

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_EQUALWEIGHT_H_ */
