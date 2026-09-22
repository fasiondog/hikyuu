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
 * cumulative multiplication way to ensure the consistency of the adjustment of the price, the
 * volume and the turnover amount.
 *
 * This indicator needs a KData context to work, it is set through the setContext() method.
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
 */
Indicator HKU_API ADJ_FACTOR();

Indicator HKU_API ADJ_FACTOR(const KData& kdata);

/**
 * @brief Calculate the adjusted open price indicator
 *
 * The open price is adjusted backward with the adjustment factor, so that the adjusted open price
 * sequence is obtained.
 * Calculation formula: ADJ_OPEN = ADJ_FACTOR * OPEN
 *
 * @return Indicator the adjusted open price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the factor management
 * system to calculate the backward proportional adjustment factor quickly
 * - In the factor management scenario, the adjustment calculation can be done efficiently by
 * updating the factor values incrementally and storing them every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily periods such as the
 * weekly and monthly periods have alignment problems and the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the factor value storage of
 * the factor management system, update_all_factors_values() should be called every day to update
 * and save the factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as
 * RECOVER_EQUAL_FORWARD; if it is not the factor management scenario, it is recommended to use
 * RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from the listing date, but
 * from the start point of the currently queried K-line data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
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
 * The high price is adjusted backward with the adjustment factor, so that the adjusted high price
 * sequence is obtained.
 * Calculation formula: ADJ_HIGH = ADJ_FACTOR * HIGH
 *
 * @return Indicator the adjusted high price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the factor management
 * system to calculate the backward proportional adjustment factor quickly
 * - In the factor management scenario, the adjustment calculation can be done efficiently by
 * updating the factor values incrementally and storing them every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily periods such as the
 * weekly and monthly periods have alignment problems and the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the factor value storage of
 * the factor management system, update_all_factors_values() should be called every day to update
 * and save the factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as
 * RECOVER_EQUAL_FORWARD; if it is not the factor management scenario, it is recommended to use
 * RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from the listing date, but
 * from the start point of the currently queried K-line data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
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
 * The low price is adjusted backward with the adjustment factor, so that the adjusted low price
 * sequence is obtained.
 * Calculation formula: ADJ_LOW = ADJ_FACTOR * LOW
 *
 * @return Indicator the adjusted low price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the factor management
 * system to calculate the backward proportional adjustment factor quickly
 * - In the factor management scenario, the adjustment calculation can be done efficiently by
 * updating the factor values incrementally and storing them every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily periods such as the
 * weekly and monthly periods have alignment problems and the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the factor value storage of
 * the factor management system, update_all_factors_values() should be called every day to update
 * and save the factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as
 * RECOVER_EQUAL_FORWARD; if it is not the factor management scenario, it is recommended to use
 * RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from the listing date, but
 * from the start point of the currently queried K-line data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
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
 * The close price is adjusted backward with the adjustment factor, so that the adjusted close price
 * sequence is obtained.
 * Calculation formula: ADJ_CLOSE = ADJ_FACTOR * CLOSE
 *
 * @return Indicator the adjusted close price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the factor management
 * system to calculate the backward proportional adjustment factor quickly
 * - In the factor management scenario, the adjustment calculation can be done efficiently by
 * updating the factor values incrementally and storing them every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily periods such as the
 * weekly and monthly periods have alignment problems and the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the factor value storage of
 * the factor management system, update_all_factors_values() should be called every day to update
 * and save the factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as
 * RECOVER_EQUAL_FORWARD; if it is not the factor management scenario, it is recommended to use
 * RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from the listing date, but
 * from the start point of the currently queried K-line data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
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
 * The volume is adjusted backward with the adjustment factor, so that the adjusted volume sequence
 * is obtained.
 * Calculation formula: ADJ_VOL = VOL / ADJ_FACTOR
 *
 * @return Indicator the adjusted volume indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the factor management
 * system to calculate the backward proportional adjustment factor quickly
 * - In the factor management scenario, the adjustment calculation can be done efficiently by
 * updating the factor values incrementally and storing them every day
 * -
 * Note: the volume adjustment uses division, which is opposite to the multiplication used by the
 * price adjustment. The reason is that when the share capital increases, the volume corresponding
 * to each share should decrease accordingly
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily periods such as the
 * weekly and monthly periods have alignment problems and the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the factor value storage of
 * the factor management system, update_all_factors_values() should be called every day to update
 * and save the factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as
 * RECOVER_EQUAL_FORWARD; if it is not the factor management scenario, it is recommended to use
 * RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from the listing date, but
 * from the start point of the currently queried K-line data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
 */
inline Indicator ADJ_VOL() {
    return VOL() / ADJ_FACTOR();
}

inline Indicator ADJ_VOL(const KData& kdata) {
    return ADJ_VOL()(kdata);
}

}  // namespace hku