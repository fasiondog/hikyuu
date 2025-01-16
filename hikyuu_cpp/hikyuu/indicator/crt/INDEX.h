/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

// 这里的大盘都指 sh000001

/** 大盘开盘价 */
Indicator HKU_API INDEXO();
Indicator HKU_API INDEXO(const KQuery& query);

/** 大盘最高价盘价 */
Indicator HKU_API INDEXH();
Indicator HKU_API INDEXH(const KQuery& query);

/** 大盘最低价 */
Indicator HKU_API INDEXL();
Indicator HKU_API INDEXL(const KQuery& query);

/** 大盘收盘价 */
Indicator HKU_API INDEXC();
Indicator HKU_API INDEXC(const KQuery& query);

/** 大盘成交额 */
Indicator HKU_API INDEXA();
Indicator HKU_API INDEXA(const KQuery& query);

/** 大盘成交量 */
Indicator HKU_API INDEXV();
Indicator HKU_API INDEXV(const KQuery& query);

/** 大盘上涨家数, 使用通达信 SH880005，可能无法用于实盘 */
Indicator HKU_API INDEXADV();
Indicator HKU_API INDEXADV(const KQuery& query);

/** 大盘下跌家数， 使用通达信 SH880005，可能无法用于实盘 */
Indicator HKU_API INDEXDEC();
Indicator HKU_API INDEXDEC(const KQuery& query);

}  // namespace hku