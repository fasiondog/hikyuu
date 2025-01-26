/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

#include "CONTEXT.h"
#include "INDEX.h"

namespace hku {

Indicator HKU_API INDEXADV() {
    KData k = getKData("SH880005", KQueryByIndex(-1));
    return CONTEXT(k.close());
}

Indicator HKU_API INDEXADV(const KQuery& query) {
    KData k = getKData("SH880005", query);
    return CONTEXT(k.close());
}

Indicator HKU_API INDEXDEC() {
    KData k = getKData("SH880005", KQueryByIndex(-1));
    return CONTEXT(k.open());
}

Indicator HKU_API INDEXDEC(const KQuery& query) {
    KData k = getKData("SH880005", query);
    return CONTEXT(k.open());
}

}  // namespace hku