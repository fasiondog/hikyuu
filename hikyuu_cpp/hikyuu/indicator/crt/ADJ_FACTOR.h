/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"
#include "KDATA.h"

namespace hku {

/**
 * @brief Calculate the adjustment factor indicator
 *
 * Calculate the backward adjustment factor sequence based on the ex-rights/ex-dividend data of the
 * stock (bonus shares, rights shares, capitalized shares, cash dividend, etc.).
 * The adjustment factor means: if 1 share was held at the listing, how many shares are held now
 * after all the bonus shares, rights shares and capitalized shares. It is calculated in a
 * cumulative multiplication way and anchored at a fixed baseline, so the factor of a given date is
 * always the same and the adjusted prices match RECOVER_EQUAL_BACKWARD.
 *
 * This indicator needs a KData context to work, it is set through the setContext() method.
 *
 * @details Design purpose of the ADJ_* family:
 * - The ADJ_* indicators are mainly designed to cooperate with the factor management system to
 *   calculate the backward proportional adjustment factor quickly
 * - In the factor management scenario, the adjustment calculation can be done efficiently by
 *   updating the factor values incrementally and storing them every day
 *
 * @warning Important limitations of the ADJ_* family:
 * - **Period limitation**: the ADJ_* indicators apply to the daily period only. Non-daily periods
 *   such as the weekly and monthly periods have alignment problems and the result may be inaccurate
 * - **Depends on factor management**: they need to be used together with the factor value storage
 *   of the factor management system, update_all_factors_values() should be called every day to
 *   update and save the factor values to guarantee the accuracy
 * - **Fixed baseline**: the factor is accumulated from the beginning of the stock's
 *   ex-rights/ex-dividend data, not from the start point of the currently queried K-line data
 * - **The K-line recovery adjusts the price only**: the volume and the turnover amount of the
 *   recovered K-line data keep their raw values (the same convention as the mainstream data
 *   sources); ADJ_VOL provides a volume consistent with the adjusted price
 *
 * @return Indicator the adjustment factor indicator object
 *
 * @par Usage example:
 * @code{.cpp}
 * // Get the adjustment factor of a stock
 * Stock stock = sm.getStock("sh000001");
 * KData kdata = stock.getKData(Query(-100));
 * Indicator adj_factor = ADJ_FACTOR();
 * adj_factor.setContext(kdata);
 * @endcode
 *
 * @see ADJ_OPEN adjusted open price
 * @see ADJ_HIGH adjusted high price
 * @see ADJ_LOW adjusted low price
 * @see ADJ_CLOSE adjusted close price
 * @see ADJ_VOL adjusted volume
 * @see RECOVER_EQUAL_BACKWARD equal backward adjustment
 */
Indicator HKU_API ADJ_FACTOR();

Indicator HKU_API ADJ_FACTOR(const KData& kdata);

/**
 * @brief Calculate the adjusted open price indicator
 *
 * Calculation formula: ADJ_OPEN = ADJ_FACTOR * OPEN; the adjusted sequence matches
 * RECOVER_EQUAL_BACKWARD. See ADJ_FACTOR for the design purpose, the limitations and the adjustment
 * convention of the ADJ_* family.
 *
 * @return Indicator the adjusted open price indicator object
 *
 * @see ADJ_FACTOR adjustment factor
 */
inline Indicator ADJ_OPEN() {
    return ADJ_FACTOR() * OPEN();
}

inline Indicator ADJ_OPEN(const KData& kdata) {
    return ADJ_OPEN()(kdata);
}

/**
 * @brief Calculate the adjusted high price indicator
 *
 * Calculation formula: ADJ_HIGH = ADJ_FACTOR * HIGH; the adjusted sequence matches
 * RECOVER_EQUAL_BACKWARD. See ADJ_FACTOR for the design purpose, the limitations and the adjustment
 * convention of the ADJ_* family.
 *
 * @return Indicator the adjusted high price indicator object
 *
 * @see ADJ_FACTOR adjustment factor
 */
inline Indicator ADJ_HIGH() {
    return ADJ_FACTOR() * HIGH();
}

inline Indicator ADJ_HIGH(const KData& kdata) {
    return ADJ_HIGH()(kdata);
}

/**
 * @brief Calculate the adjusted low price indicator
 *
 * Calculation formula: ADJ_LOW = ADJ_FACTOR * LOW; the adjusted sequence matches
 * RECOVER_EQUAL_BACKWARD. See ADJ_FACTOR for the design purpose, the limitations and the adjustment
 * convention of the ADJ_* family.
 *
 * @return Indicator the adjusted low price indicator object
 *
 * @see ADJ_FACTOR adjustment factor
 */
inline Indicator ADJ_LOW() {
    return ADJ_FACTOR() * LOW();
}

inline Indicator ADJ_LOW(const KData& kdata) {
    return ADJ_LOW()(kdata);
}

/**
 * @brief Calculate the adjusted close price indicator
 *
 * Calculation formula: ADJ_CLOSE = ADJ_FACTOR * CLOSE; the adjusted sequence matches
 * RECOVER_EQUAL_BACKWARD. See ADJ_FACTOR for the design purpose, the limitations and the adjustment
 * convention of the ADJ_* family.
 *
 * @return Indicator the adjusted close price indicator object
 *
 * @see ADJ_FACTOR adjustment factor
 */
inline Indicator ADJ_CLOSE() {
    return ADJ_FACTOR() * CLOSE();
}

inline Indicator ADJ_CLOSE(const KData& kdata) {
    return ADJ_CLOSE()(kdata);
}

/**
 * @brief Calculate the adjusted volume indicator
 *
 * The volume is divided by the same factor that the adjusted price is multiplied by, so that the
 * turnover amount (the adjusted price x the adjusted volume) stays the raw amount.
 * Calculation formula: ADJ_VOL = VOL / ADJ_FACTOR
 *
 * @details Note: the volume moves in the direction opposite to the price (the factor is the
 * reciprocal); this is the volume convention of the factor management system and differs from a
 * volume re-expressed in the share terms only (which ignores the cash dividend, since a cash
 * dividend changes the price but not the share count). The K-line recovery adjusts the price only
 * and leaves the volume untouched, so use this indicator when a volume consistent with the adjusted
 * price is required. See ADJ_FACTOR for the design purpose, the limitations and the adjustment
 * convention of the ADJ_* family.
 *
 * @return Indicator the adjusted volume indicator object
 *
 * @see ADJ_FACTOR adjustment factor
 */
inline Indicator ADJ_VOL() {
    return VOL() / ADJ_FACTOR();
}

inline Indicator ADJ_VOL(const KData& kdata) {
    return ADJ_VOL()(kdata);
}

}  // namespace hku
