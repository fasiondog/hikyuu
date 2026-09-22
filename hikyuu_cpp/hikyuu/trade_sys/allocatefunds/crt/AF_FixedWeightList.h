/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#pragma once

#include "../AllocateFundsBase.h"

namespace hku {

/**
 * @brief Fixed proportion asset allocation
 * @param weights the given asset proportion list
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HKU_API AF_FixedWeightList(const vector<double>& weights);

} /* namespace hku */
