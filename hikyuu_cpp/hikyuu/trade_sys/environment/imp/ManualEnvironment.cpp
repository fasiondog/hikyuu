/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ManualEnvironment)
#endif

namespace hku {

ManualEnvironment::ManualEnvironment() : EnvironmentBase("EV_Manual") {}

void ManualEnvironment::_calculate() {}

EVPtr HKU_API EV_Manual() {
    return make_shared<ManualEnvironment>();
}

}  // namespace hku