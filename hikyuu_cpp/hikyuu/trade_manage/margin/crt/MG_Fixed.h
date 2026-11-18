/*
 * MG_Fixed.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_MG_FIXED_H_
#define TRADE_MANAGE_CRT_MG_FIXED_H_

#include "../MarginBase.h"

namespace hku {

/**
 * Create a fixed margin model applied to every security of the account
 * @param rate opening margin ratio, (0, 1], 0.6 by default
 * @param rate_short short opening ratio, 0 means following rate
 * @param maintain_rate maintain margin ratio, 0 means the same as the opening margin
 * @param margin_per_lot margin of one lot, 0 means using the rate mode
 * @see FixedMargin
 * @ingroup Margin
 */
MarginPtr HKU_API MG_Fixed(double rate = 0.6, double rate_short = 0.0, double maintain_rate = 0.0,
                           double margin_per_lot = 0.0);

void registerPart_MG_Fixed();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_MG_FIXED_H_ */
