/*
 * MG_None.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_MG_NONE_H_
#define TRADE_MANAGE_CRT_MG_NONE_H_

#include "../MarginBase.h"

namespace hku {

/**
 * Create a plain cash margin model, i.e. no leverage at all
 * @see NoMargin
 * @ingroup Margin
 */
MarginPtr HKU_API MG_None();

void registerPart_MG_None();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_MG_NONE_H_ */
