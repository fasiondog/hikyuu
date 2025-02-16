/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-17
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_CRT_AF_NOTHING_H_
#define TRADE_SYS_ALLOCATEFUNDS_CRT_AF_NOTHING_H_

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 创建不做资产分配的资产分配器
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_Nothing();

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_CRT_AF_NOTHING_H_ */
