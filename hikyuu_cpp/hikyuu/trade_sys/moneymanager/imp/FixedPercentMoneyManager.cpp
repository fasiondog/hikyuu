/*
 * PercentRiskMoneyManager.cpp
 *
 *  Created on: 2015-4-4
 *      Author: fasiondog
 */

#include "FixedPercentMoneyManager.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentMoneyManager)
#endif

namespace hku {

FixedPercentMoneyManager::FixedPercentMoneyManager() : MoneyManagerBase("MM_FixedPercent") {
    setParam<double>("p", 0.02);  // Fraction of available cash risked per trade
}

FixedPercentMoneyManager::~FixedPercentMoneyManager() {}

void FixedPercentMoneyManager::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>("p");
        HKU_ASSERT(p > 0 && p <= 1.0);
    }
}

double FixedPercentMoneyManager::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                               price_t price, price_t risk, SystemPart from) {
    double p = getParam<double>("p");
    return m_tm->cash(datetime, m_query.kType()) * p / risk;
}

double FixedPercentMoneyManager::_getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                                     price_t price, price_t risk, SystemPart from) {
    // Base the risk budget on the net assets (the Van Tharp equity basis): the borrowed proceeds
    // in cash are a liability and must not inflate the short size
    double p = getParam<double>("p");
    return m_tm->getFunds(datetime, m_query.kType()).net_assets() * p / risk;
}

MoneyManagerPtr HKU_API MM_FixedPercent(double p) {
    MoneyManagerPtr ptr = make_shared<FixedPercentMoneyManager>();
    ptr->setParam<double>("p", p);
    return ptr;
}

void registerPart_MM_FixedPercent() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.mm.fixedpercent"), void());
    PartMeta meta;
    meta.type = PartType::MM;
    meta.name = "MM_FixedPercent";
    meta.full_name = "core.mm.fixedpercent";
    meta.params.push_back(ParamMeta{.name = "p", .type = "double", .required = true});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return MM_FixedPercent(sc.get<double>("p"));
    });
}

} /* namespace hku */
