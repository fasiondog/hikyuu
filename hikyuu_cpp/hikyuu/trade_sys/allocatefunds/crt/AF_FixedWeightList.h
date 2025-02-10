/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 */

#pragma once

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief 固定比例资产分配
 * @param weights 指定的资产比例列表
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeightList(const vector<double>& weights);

} /* namespace hku */
