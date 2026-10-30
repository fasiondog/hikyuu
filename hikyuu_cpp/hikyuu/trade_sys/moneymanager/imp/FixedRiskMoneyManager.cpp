/*
 * FixedRiskMM.cpp
 *
 *  Created on: 2016-5-1
 *      Author: Administrator
 */

#include "FixedRiskMoneyManager.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedRiskMoneyManager)
#endif

namespace hku {

FixedRiskMoneyManager::FixedRiskMoneyManager() : MoneyManagerBase("MM_FixedRisk") {
    setParam<double>("risk", 1000.00);
}

FixedRiskMoneyManager::~FixedRiskMoneyManager() {}

void FixedRiskMoneyManager::_checkParam(const string& name) const {
    if ("risk" == name) {
        double risk = getParam<double>("risk");
        HKU_ASSERT(risk > 0.0);
    }
}

double FixedRiskMoneyManager::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    return getParam<double>("risk") / risk;
}

double FixedRiskMoneyManager::_getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                                  price_t price, price_t risk, SystemPart from) {
    // The same fixed-risk sizing as the long buy; the short risk is the stop-loss price minus
    // the short entry price
    return getParam<double>("risk") / risk;
}

MoneyManagerPtr HKU_API MM_FixedRisk(double risk) {
    MoneyManagerPtr p = make_shared<FixedRiskMoneyManager>();
    p->setParam<double>("risk", risk);
    return p;
}

void registerPart_MM_FixedRisk() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.mm.fixedrisk"), void());
    PartMeta meta;
    meta.type = PartType::MM;
    meta.name = "MM_FixedRisk";
    meta.full_name = "core.mm.fixedrisk";
    meta.params.push_back(ParamMeta{.name = "risk", .type = "double", .default_value = "1000.00"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return MM_FixedRisk(sc.get<double>("risk"));
    });
}

} /* namespace hku */
