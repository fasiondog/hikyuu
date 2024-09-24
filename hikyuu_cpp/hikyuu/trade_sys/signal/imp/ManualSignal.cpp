/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ManualSignal)
#endif

namespace hku {

ManualSignal::ManualSignal() : SignalBase("SG_Manual") {}

void ManualSignal::_calculate(const KData&) {}

SignalPtr HKU_API SG_Manual() {
    return make_shared<ManualSignal>();
}

}  // namespace hku