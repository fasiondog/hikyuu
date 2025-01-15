/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/** 大盘开盘价 */
inline Indicator INDEXO() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.open());
}

inline Indicator INDEXO(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.open());
}

/** 大盘最高价盘价 */
inline Indicator INDEXH() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.high());
}

inline Indicator INDEXH(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.high());
}

/** 大盘最低价 */
inline Indicator INDEXL() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.low());
}

inline Indicator INDEXL(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.low());
}

/** 大盘收盘价 */
inline Indicator INDEXC() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.close());
}

inline Indicator INDEXC(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.close());
}

/** 大盘成交额 */
inline Indicator INDEXA() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.amo());
}

inline Indicator INDEXA(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.amo());
}

/** 大盘成交量 */
inline Indicator INDEXV() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.vol());
}

inline Indicator INDEXV(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.vol());
}

#if 0
/** 大盘上涨家数 */
Indicator HKU_API INDEXADV();

/** 大盘下跌家数 */
Indicator HKU_API INDEXDEC();
#endif

}  // namespace hku