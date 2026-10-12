/*
 * TIMELINE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TIMELINE_H_
#define INDICATOR_CRT_TIMELINE_H_

#include "../Indicator.h"

namespace hku {

/**
 * Get the time-line price data of the context query range
 * @details
 * <pre>
 * Usage: TIMELINE(k) or TIMELINE()(k), where k is the context KData.
 * The result is the raw time-line series, so its length follows the time-line data, not the
 * context K-line bar count.
 * </pre>
 * @note Leaf-only indicator: bind a context and read the series directly (e.g. for plotting).
 *       Do not feed it into other indicators: when the time-line length differs from the context
 *       bar count, the framework context-length contract rejects such usage with an exception;
 *       when they happen to be equal, the result would be a silent cross-purpose mixing of
 *       time-line points and K-line bars. The date accessors (getDatetime, getDatetimeList,
 *       operator[](Datetime)) of the result return the context K-line dates, not the time-line
 *       point timestamps.
 * @ingroup Indicator
 */
Indicator HKU_API TIMELINE();
Indicator HKU_API TIMELINE(const KData&);

}  // namespace hku

#endif /* INDICATOR_CRT_TIMELINE_H_ */
