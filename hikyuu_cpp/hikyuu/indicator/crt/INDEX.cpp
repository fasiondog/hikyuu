/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

#include "CONTEXT.h"
#include "INDEX.h"

namespace hku {

static KData get_advance_decline_kdata(const KQuery& query) {
    KData k = getKData("SH880005", query);
    HKU_WARN_IF(k.empty(),
                "TDX advance/decline index SH880005 has no data! Check whether the data package "
                "contains it and whether the engine is initialized before building the formula.");
    return k;
}

// TDX SH880005 encodes the rising stock count as close and the falling stock count as open
Indicator HKU_API INDEXADV() {
    return CONTEXT(get_advance_decline_kdata(KQueryByIndex(-1)).close());
}

Indicator HKU_API INDEXADV(const KQuery& query) {
    return CONTEXT(get_advance_decline_kdata(query).close());
}

Indicator HKU_API INDEXDEC() {
    return CONTEXT(get_advance_decline_kdata(KQueryByIndex(-1)).open());
}

Indicator HKU_API INDEXDEC(const KQuery& query) {
    return CONTEXT(get_advance_decline_kdata(query).open());
}

}  // namespace hku