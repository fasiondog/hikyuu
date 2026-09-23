/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

// The broad market here always refers to sh000001

/** The corresponding broad market open price: Shanghai Composite Index, Shenzhen Component Index,
 *  STAR 50 and ChiNext Index */
Indicator HKU_API INDEXO(bool fill_null = true);
inline Indicator INDEXO(const KData& k, bool fill_null = true) {
    return INDEXO(fill_null)(k);
}

/** The corresponding broad market high price: Shanghai Composite Index, Shenzhen Component Index,
 *  STAR 50 and ChiNext Index */
Indicator HKU_API INDEXH(bool fill_null = true);
inline Indicator INDEXH(const KData& k, bool fill_null = true) {
    return INDEXH(fill_null)(k);
}

/** The corresponding broad market low price: Shanghai Composite Index, Shenzhen Component Index,
 *  STAR 50 and ChiNext Index */
Indicator HKU_API INDEXL(bool fill_null = true);
inline Indicator INDEXL(const KData& k, bool fill_null = true) {
    return INDEXL(fill_null)(k);
}

/** The corresponding broad market close price: Shanghai Composite Index, Shenzhen Component Index,
 *  STAR 50 and ChiNext Index */
Indicator HKU_API INDEXC(bool fill_null = true);
inline Indicator INDEXC(const KData& k, bool fill_null = true) {
    return INDEXC(fill_null)(k);
}

/** The corresponding broad market turnover amount: Shanghai Composite Index, Shenzhen Component
 *  Index, STAR 50 and ChiNext Index */
Indicator HKU_API INDEXA(bool fill_null = true);
inline Indicator INDEXA(const KData& k, bool fill_null = true) {
    return INDEXA(fill_null)(k);
}

/** The corresponding broad market volume: Shanghai Composite Index, Shenzhen Component Index,
 *  STAR 50 and ChiNext Index */
Indicator HKU_API INDEXV(bool fill_null = true);
inline Indicator INDEXV(const KData& k, bool fill_null = true) {
    return INDEXV(fill_null)(k);
}

/** Number of the rising stocks in the broad market, using the TDX SH880005, it may not be usable
 *  for live trading */
Indicator HKU_API INDEXADV();
Indicator HKU_API INDEXADV(const KQuery& query);

/** Number of the falling stocks in the broad market, using the TDX SH880005, it may not be usable
 *  for live trading */
Indicator HKU_API INDEXDEC();
Indicator HKU_API INDEXDEC(const KQuery& query);

}  // namespace hku