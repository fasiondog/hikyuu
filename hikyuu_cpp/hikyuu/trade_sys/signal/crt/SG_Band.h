/*
 * SG_Band.h
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */
#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_BAND_H_
#define TRADE_SYS_SIGNAL_CRT_SG_BAND_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

SignalPtr HKU_API SG_Band(const Indicator& sig, price_t lower, price_t upper);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_ */
