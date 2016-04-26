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
Indicator HKU_API KDATA();
Indicator HKU_API KDATA(const KData&);
Indicator HKU_API KDATA(const Indicator&);

/**
 * 包装KData的开盘价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API OPEN();
Indicator HKU_API OPEN(const KData&);
Indicator HKU_API OPEN(const Indicator&);

/**
 * 包装KData的最高价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API HIGH();
Indicator HKU_API HIGH(const KData&);
Indicator HKU_API HIGH(const Indicator&);

/**
 * 包装KData的最低价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API LOW();
Indicator HKU_API LOW(const KData&);
Indicator HKU_API LOW(const Indicator&);

/**
 * 包装KData的收盘价成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API CLOSE();
Indicator HKU_API CLOSE(const KData&);
Indicator HKU_API CLOSE(const Indicator&);

/**
 * 包装KData的成交金额成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API AMO();
Indicator HKU_API AMO(const KData&);
Indicator HKU_API AMO(const Indicator&);

/**
 * 包装KData的成交量成Indicator，用于其他指标计算
 * @ingroup Indicator
 */
Indicator HKU_API VOL();
Indicator HKU_API VOL(const KData&);
Indicator HKU_API VOL(const Indicator&);

/**
 * 根据字符串选择返回KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL
 * @param kdata K线数据
 * @param kpart KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
 * @see KDATA, OPEN, HIGH, LOW, CLOSE, AMO, VOL
 * @ingroup Indicator
 */
Indicator HKU_API KDATA_PART(const string& kpart);
Indicator HKU_API KDATA_PART(const KData& kdata, const string& kpart);
Indicator HKU_API KDATA_PART(const Indicator& kdata, const string& kpart);

} /* namespace */


