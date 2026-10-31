/*
 * NoGoalProfitGoal.cpp
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#include "NoGoalProfitGoal.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NoGoalProfitGoal)
#endif

namespace hku {

NoGoalProfitGoal::NoGoalProfitGoal() : ProfitGoalBase("PG_NoGoal") {}

NoGoalProfitGoal::~NoGoalProfitGoal() {}

price_t NoGoalProfitGoal::_getGoal(const Datetime& datetime, price_t price) {
    return Null<price_t>();
}

ProfitGoalPtr HKU_API PG_NoGoal() {
    return make_shared<NoGoalProfitGoal>();
}

void registerPart_PG_NoGoal() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.pg.nogoal"), void());
    PartMeta meta;
    meta.type = PartType::PG;
    meta.name = "PG_NoGoal";
    meta.full_name = "core.pg.nogoal";
    reg.registerPart(
      meta, [](const Parameter& sc, const PartParamMap& pp) -> Part { return PG_NoGoal(); });
}

} /* namespace hku */
