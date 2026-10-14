/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * Uniform distribution random price slippage algorithm: the sampled value is folded to a
 * non-negative amplitude and applied in the unfavorable direction, so the buy price is the plan
 * price plus the amplitude and the sell price is the plan price minus the amplitude. Thus
 * [min_value, max_value] is the amplitude range (the sign of the sample is ignored), not a
 * symmetric +/- offset.
 * @param min_value the lower limit of the offset amplitude range
 * @param max_value the upper limit of the offset amplitude range
 * @return
 */
SlippagePtr HKU_API SP_Uniform(double min_value = -0.05, double max_value = 0.05);

} /* namespace hku */
