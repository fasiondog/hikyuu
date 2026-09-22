/*
 * IKDATA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#include "../Indicator.h"

namespace hku {

/**
 * Get the year, month and day of this period since 1900. Usage: DATE. For example, the function
 * returns 1000101, meaning January 1, 2000.

 * @ingroup Indicator
 */
Indicator HKU_API DATE();
Indicator HKU_API DATE(const KData&);

/**
 * Get the hour, minute and second of this period. Usage: TIME. The valid value range of the
 * function is (000000-235959).

 * @ingroup Indicator
 */
Indicator HKU_API TIME();
Indicator HKU_API TIME(const KData&);

/**
 * Get the year of this period.

 * @ingroup Indicator
 */
Indicator HKU_API YEAR();
Indicator HKU_API YEAR(const KData&);

/**
 * Get the month of this period. Usage: MONTH. The valid value range of the function is (1-12).

 * @ingroup Indicator
 */
Indicator HKU_API MONTH();
Indicator HKU_API MONTH(const KData&);

/**
 * Get the day of the week of this period. Usage: WEEK. The valid value range of the function is
 * (0-6), 0 means Sunday.

 * @ingroup Indicator
 */
Indicator HKU_API WEEK();
Indicator HKU_API WEEK(const KData&);

/**
 * Get the day of this period. Usage: DAY. The valid value range of the function is (1-31).

 * @ingroup Indicator
 */
Indicator HKU_API DAY();
Indicator HKU_API DAY(const KData&);

/**
 * Get the hour of this period. Usage: HOUR. The valid value range of the function is (0-23); the
 * value is 0 for the daily line and longer analysis periods.

 * @ingroup Indicator
 */
Indicator HKU_API HOUR();
Indicator HKU_API HOUR(const KData&);

/**
 * Get the minute of this period. Usage: MINUTE. The valid value range of the function is (0-59);
 * the value is 0 for the daily line and longer analysis periods.

 * @ingroup Indicator
 */
Indicator HKU_API MINUTE();
Indicator HKU_API MINUTE(const KData&);

}  // namespace hku
