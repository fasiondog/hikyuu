/*
 * SG_Band.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */
#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_BAND_H_
#define TRADE_SYS_SIGNAL_CRT_SG_BAND_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Indicator band indicator: buy when the indicator exceeds the upper band, and sell when the
 * indicator is lower than the lower band.
 * @note It is suitable for the RSI-like indicators with an absolute value range
 * @param ind indicator
 * @param lower lower band
 * @param upper upper band
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Band(const Indicator& ind, price_t lower, price_t upper);

/**
 * Indicator band indicator: buy when the indicator exceeds the upper band indicator, and sell when
 * the indicator is lower than the lower band indicator.
 * @param ind indicator
 * @param lower lower band indicator
 * @param upper upper band indicator
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Band(const Indicator& ind, const Indicator& lower, const Indicator& upper);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_ */
