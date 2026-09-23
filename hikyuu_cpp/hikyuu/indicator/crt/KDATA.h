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
 * Wrap KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HKU_API KDATA();
Indicator HKU_API KDATA(const KData&);

/**
 * Wrap the open price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HKU_API OPEN();
Indicator HKU_API OPEN(const KData&);

/**
 * Wrap the high price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HKU_API HIGH();
Indicator HKU_API HIGH(const KData&);

/**
 * Wrap the low price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HKU_API LOW();
Indicator HKU_API LOW(const KData&);

/**
 * Wrap the close price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HKU_API CLOSE();
Indicator HKU_API CLOSE(const KData&);

/**
 * Wrap the turnover amount of KData into an Indicator, used for the calculation of the other
 * indicators
 * @ingroup Indicator
 */
Indicator HKU_API AMO();
Indicator HKU_API AMO(const KData&);

/**
 * Wrap the volume of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HKU_API VOL();
Indicator HKU_API VOL(const KData&);

/**
 * Return KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL according to the string
 * @param kdata K-line data
 * @param kpart KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
 * @see KDATA, OPEN, HIGH, LOW, CLOSE, AMO, VOL
 * @ingroup Indicator
 */
Indicator HKU_API KDATA_PART(const KData& kdata, const string& kpart);
Indicator HKU_API KDATA_PART(const string& kpart);

}  // namespace hku
