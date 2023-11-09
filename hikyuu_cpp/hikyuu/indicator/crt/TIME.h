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
 * 取得该周期从1900以来的年月日。用法: DATE 例如函数返回1000101，表示2000年1月1日。
 * @ingroup Indicator
 */
Indicator HKU_API DATE();
Indicator HKU_API DATE(const KData&);

/**
 * 取得该周期的时分秒。用法: TIME 函数返回有效值范围为(000000-235959)。
 * @ingroup Indicator
 */
Indicator HKU_API TIME();
Indicator HKU_API TIME(const KData&);

/**
 * 取得该周期的年份。
 * @ingroup Indicator
 */
Indicator HKU_API YEAR();
Indicator HKU_API YEAR(const KData&);

/**
 * 取得该周期的月份。用法: MONTH 函数返回有效值范围为(1-12)。
 * @ingroup Indicator
 */
Indicator HKU_API MONTH();
Indicator HKU_API MONTH(const KData&);

/**
 * 取得该周期的星期数。用法：WEEK 函数返回有效值范围为(0-6)，0表示星期天。
 * @ingroup Indicator
 */
Indicator HKU_API WEEK();
Indicator HKU_API WEEK(const KData&);

/**
 * 取得该周期的日期。用法: DAY 函数返回有效值范围为(1-31)。
 * @ingroup Indicator
 */
Indicator HKU_API DAY();
Indicator HKU_API DAY(const KData&);

/**
 * 取得该周期的小时数。用法：HOUR 函数返回有效值范围为(0-23)，对于日线及更长的分析周期值为0。
 * @ingroup Indicator
 */
Indicator HKU_API HOUR();
Indicator HKU_API HOUR(const KData&);

/**
 * 取得该周期的分钟数。用法：MINUTE 函数返回有效值范围为(0-59)，对于日线及更长的分析周期值为0。
 * @ingroup Indicator
 */
Indicator HKU_API MINUTE();
Indicator HKU_API MINUTE(const KData&);

}  // namespace hku
