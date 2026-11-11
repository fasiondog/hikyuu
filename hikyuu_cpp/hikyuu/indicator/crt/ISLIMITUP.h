/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * @brief Indicator for judging whether a stock is limit up
 * @ingroup Indicator
 * @details Judge whether the close price of the day reaches the limit-up price according to the
 * different stock types:
 * - Ordinary A-share stock: the limit-up range is 10%
 * - Beijing Stock Exchange stock: the limit-up range is 30%
 * - ChiNext / STAR Market stock: the limit-up range is 20%
 * - ST stock: the limit-up range is 5%, but it is not handled yet because the historical date
 * information of the ST identifier is missing
 *
 * Limit-up judgment logic: close price of the day >= the previous close × (1 + limit-up range),
 * where the previous close is the raw close of the previous trading day
 *
 * Notes:
 * - The first K-line data is marked as discard because the previous day data is missing
 * - An unsupported stock type returns the non-limit-up state by default
 * - The calculation result is a boolean value: 1 means limit up, 0 means not limit up
 * - The ranges follow the current rules of each board, without date segmentation: the ChiNext
 *   stocks were limited to 10% before 2020-08-24 and a new listing has no range limit in its
 *   first days, both of which are judged by the current range
 * - The judgment runs on the raw (NO_RECOVER) closes: a recovered context is re-fetched
 *   internally with the same query, so the prices stay on the exchange scale
 * - On an ex-rights/ex-dividend day the exchange takes the ex-rights reference price instead of
 *   the record date close as the previous close: (record date close - cash dividend + rights
 *   issue price × share change ratio) / (1 + share change ratio); the limit price is based on it
 *
 * <pre>
 * Example:
 * @code
 * // Create the limit-up judgment indicator
 * Indicator limit_up = ISLIMITUP();
 *
 * // Judge the given K-line data
 * KData kdata = sm["sh000001"].getKData(KQuery(-100));
 * Indicator result = ISLIMITUP(kdata);
 *
 * @endcode
 * </pre>
 *
 * @return Indicator the limit-up judgment indicator instance
 */
Indicator HKU_API ISLIMITUP();

/**
 * @brief Judge whether the stock in the given K-line data is limit up
 * @ingroup Indicator
 * @param k K-line data
 * @return Indicator the limit-up judgment indicator instance
 * @see ISLIMITUP()
 */
Indicator HKU_API ISLIMITUP(const KData& k);

inline void registerPart_ISLIMITUP() {
    registerIndicatorPart("core.ind.islimitup", [] { return ISLIMITUP(); });
}

}  // namespace hku