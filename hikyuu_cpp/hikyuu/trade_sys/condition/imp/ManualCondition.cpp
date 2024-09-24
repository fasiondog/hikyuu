/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ManualCondition)
#endif

namespace hku {

ManualCondition::ManualCondition() : ConditionBase("CN_Manual") {}

void ManualCondition::_calculate() {}

CNPtr HKU_API CN_Manual() {
    return make_shared<ManualCondition>();
}

}  // namespace hku