/*
 * MG_ByType.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_MG_BY_TYPE_H_
#define TRADE_MANAGE_CRT_MG_BY_TYPE_H_

#include "../MarginBase.h"

namespace hku {

/**
 * Create a margin model keyed by security type or market
 * @param rates "key:ratio" list; all-digit keys are security types, others are market codes
 * @param maintain_rate maintain margin ratio, 0 means the same as the opening margin
 * @param default_rate ratio when no key matches, (0, 1], 0.6 by default
 * @param collateral_rates "key:ratio" list of collateral conversion rates, same key syntax; empty
 *        counts every holding in full
 * @param default_collateral_rate conversion rate when no collateral key matches, (0, 1], 1.0 by default
 * @see ByTypeMargin
 * @ingroup Margin
 */
MarginPtr HKU_API MG_ByType(const string& rates = "", double maintain_rate = 0.0,
                            double default_rate = 0.6, const string& collateral_rates = "",
                            double default_collateral_rate = 1.0);

void registerPart_MG_ByType();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_MG_BY_TYPE_H_ */
