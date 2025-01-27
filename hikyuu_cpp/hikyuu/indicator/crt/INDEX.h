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

/** 对应的大盘开盘价,分别是上证指数,深证成指,科创50,创业板指 */
Indicator HKU_API INDEXO(bool fill_null = true);
inline Indicator INDEXO(const KData& k, bool fill_null = true) {
    return INDEXO(fill_null)(k);
}

/** 对应的大盘最高价,分别是上证指数,深证成指,科创50,创业板指 */
Indicator HKU_API INDEXH(bool fill_null = true);
inline Indicator INDEXH(const KData& k, bool fill_null = true) {
    return INDEXH(fill_null)(k);
}

/** 对应的大盘最低价,分别是上证指数,深证成指,科创50,创业板指 */
Indicator HKU_API INDEXL(bool fill_null = true);
inline Indicator INDEXL(const KData& k, bool fill_null = true) {
    return INDEXL(fill_null)(k);
}

/** 对应的大盘收盘价,分别是上证指数,深证成指,科创50,创业板指 */
Indicator HKU_API INDEXC(bool fill_null = true);
inline Indicator INDEXC(const KData& k, bool fill_null = true) {
    return INDEXC(fill_null)(k);
}

/** 对应的大盘成交金额,分别是上证指数,深证成指,科创50,创业板指 */
Indicator HKU_API INDEXA(bool fill_null = true);
inline Indicator INDEXA(const KData& k, bool fill_null = true) {
    return INDEXA(fill_null)(k);
}

/** 对应的大盘成交量,分别是上证指数,深证成指,科创50,创业板指 */
Indicator HKU_API INDEXV(bool fill_null = true);
inline Indicator INDEXV(const KData& k, bool fill_null = true) {
    return INDEXV(fill_null)(k);
}

/** 大盘上涨家数, 使用通达信 SH880005，可能无法用于实盘 */
Indicator HKU_API INDEXADV();
Indicator HKU_API INDEXADV(const KQuery& query);

/** 大盘下跌家数， 使用通达信 SH880005，可能无法用于实盘 */
Indicator HKU_API INDEXDEC();
Indicator HKU_API INDEXDEC(const KQuery& query);

}  // namespace hku