/*
 * STKTYPE.h
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_STKTYPE_H_
#define INDICATOR_CRT_STKTYPE_H_

#include "../Indicator.h"

namespace hku {

/**
 * Get the stock type indicator

 * 
 * Return the type value of the current stock (the StockType enumeration value)

 * 
 * @param k K-line data context (optional)

 * @return the indicator object, the value at every position is the type value of the stock

 * @ingroup Indicator
 * 
 * @par Example:

 * @code
 * // Use the default context

 * auto stktype = STKTYPE();
 * 
 * // The given K-line data

 * Stock stock = sm.getStock("sh000001");
 * KData kdata = stock.getKData(KQuery(0, 100));
 * auto stktype = STKTYPE(kdata);
 * @endcode
 */
Indicator HKU_API STKTYPE();
Indicator HKU_API STKTYPE(const KData& k);

}  // namespace hku

#endif /* INDICATOR_CRT_STKTYPE_H_ */
