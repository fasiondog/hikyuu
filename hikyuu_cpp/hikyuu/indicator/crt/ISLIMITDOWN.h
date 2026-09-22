/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#include "../Indicator.h"

namespace hku {

/**
 * @brief Indicator for judging whether a stock is limit down
 * @ingroup Indicator
 * @details Judge whether the close price of the day reaches the limit-down price according to the
 * different stock types:
 * - Ordinary A-share stock: the limit-down range is 10%
 * - Beijing Stock Exchange stock: the limit-down range is 30%
 * - ChiNext / STAR Market stock: the limit-down range is 20%
 * - ST stock: the limit-down range is 5%, but it is not handled yet because the historical date
 * information of the ST identifier is missing
 *
 * Limit-down judgment logic: close price of the day <= close price of the previous day × (1 -
 * limit-down range)
 *
 * Notes:
 * - The first K-line data is marked as discard because the previous day data is missing
 * - An unsupported stock type returns the non-limit-down state by default
 * - The calculation result is a boolean value: 1 means limit down, 0 means not limit down
 *
 * <pre>
 * Example:
 * @code
 * // Create the limit-down judgment indicator
 * Indicator limit_down = ISLIMITDOWN();
 *
 * // Judge the given K-line data
 * KData kdata = sm["sh000001"].getKData(KQuery(-100));
 * Indicator result = ISLIMITDOWN(kdata);
 *
 * // Get the judgment result
 * for (size_t i = 0; i < result.size(); ++i) {
 *     if (result[i] == 1.0) {
 *         cout << "day " << i << " is limit down" << endl;
 *     }
 * }
 * @endcode
 * </pre>
 *
 * @return Indicator the limit-down judgment indicator instance
 */
Indicator HKU_API ISLIMITDOWN();

/**
 * @brief Judge whether the stock in the given K-line data is limit down
 * @ingroup Indicator
 * @param k K-line data
 * @return Indicator the limit-down judgment indicator instance
 * @see ISLIMITDOWN()
 */
Indicator HKU_API ISLIMITDOWN(const KData& k);

}  // namespace hku