/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

#include "CONTEXT.h"
#include "INDEX.h"

namespace hku {

Indicator HKU_API INDEXO() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.open());
}

Indicator HKU_API INDEXO(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.open());
}

Indicator HKU_API INDEXH() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.high());
}

Indicator HKU_API INDEXH(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.high());
}

Indicator HKU_API INDEXL() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.low());
}

Indicator HKU_API INDEXL(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.low());
}

Indicator HKU_API INDEXC() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.close());
}

Indicator HKU_API INDEXC(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.close());
}

Indicator HKU_API INDEXA() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.amo());
}

Indicator HKU_API INDEXA(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.amo());
}

Indicator HKU_API INDEXV() {
    KData k = getKData("SH000001", KQueryByIndex(-1));
    return CONTEXT(k.vol());
}

Indicator HKU_API INDEXV(const KQuery& query) {
    KData k = getKData("SH000001", query);
    return CONTEXT(k.vol());
}

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

}