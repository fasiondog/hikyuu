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

/**
 * 指标区间指示器, 当指标超过上轨时，买入；当指标低于下轨时，卖出。
 * @note 适用于 RSI 类，有绝对值区间的指标
 * @param ind 指标
 * @param lower 下轨
 * @param upper 上轨
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Band(const Indicator& ind, price_t lower, price_t upper);

/**
 * 指标区间指示器, 当指标超过上轨指标时，买入；当指标低于下轨指标时，卖出。
 * @param ind 指标
 * @param lower 下轨指标
 * @param upper 上轨指标
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Band(const Indicator& ind, const Indicator& lower, const Indicator& upper);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_ */
