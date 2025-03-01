/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * 创建 MultiFactor 评分权重的资产分配算法实例，即直接以SE返回的评分作为权重。
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_MultiFactor();

}  // namespace hku