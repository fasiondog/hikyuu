/*
 * IKDATA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "../Indicator.h"

namespace hku {

/**
 * 包装KData成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API KDATA(const KData&);

/**
 * 包装KData的开盘价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API OPEN(const KData&);

/**
 * 包装KData的最高价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API HIGH(const KData&);

/**
 * 包装KData的最低价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API LOW(const KData&);

/**
 * 包装KData的收盘价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API CLOSE(const KData&);

/**
 * 包装KData的成交金额成Indicator，用于其他指标计算，同AMO
 * @see AMO
 * @ingroup Indicator
 */
Indicator HKU_API TRANSAMOUNT(const KData&);

/**
 * 包装KData的成交量成Indicator，用于其他指标计算，同VOL
 * @see VOL
 * @ingroup Indicator
 */
Indicator HKU_API TRANSCOUNT(const KData&);

/**
 * 包装KData的成交金额成Indicator，用于其他指标计算, 同TRANSAMOUNT
 * @see TRANSAMOUNT
 * @ingroup Indicator
 */
Indicator HKU_API AMO(const KData&);

/**
 * 包装KData的成交量成Indicator，用于其他指标计算，同TRANSCOUNT
 * @see TRANSCOUNT
 * @ingroup Indicator
 */
Indicator HKU_API VOL(const KData&);

/**
 * 根据字符串选择返回KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL
 * @param kdata K线数据
 * @param part KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL|TRANSAMOUNT|TRANSCOUNT
 * @see KDATA, OPEN, HIGH, LOW, CLOSE, AMO, VOL
 * @ingroup Indicator
 */
Indicator HKU_API KDATA_PART(const KData& kdata, const string& part);

} /* namespace */


