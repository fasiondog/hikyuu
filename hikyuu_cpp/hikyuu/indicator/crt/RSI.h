/*
 * RSI.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */
#pragma once
#ifndef INDICATOR_CRT_RSI_H_
#define INDICATOR_CRT_RSI_H_

#include "../Indicator.h"
#include "REF.h"
#include "EMA.h"
#include "CVAL.h"

namespace hku {

/**
 * Relative strength index
 * @ingroup Indicator
 */
Indicator HKU_API RSI(int n = 14);
Indicator HKU_API RSI(const Indicator& data, int n = 14);

}  // namespace hku

#endif /* INDICATOR_CRT_RSI_H_ */
