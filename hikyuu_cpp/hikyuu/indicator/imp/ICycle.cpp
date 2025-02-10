/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#include "ICycle.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICycle)
#endif

namespace hku {

ICycle::ICycle() : IndicatorImp("CYCLE", 1) {
    _initParams();
}

ICycle::ICycle(const KData& k) : IndicatorImp("CYCLE", 1) {
    _initParams();
    setParam<KData>("kdata", k);
    ICycle::_calculate(Indicator());
}

ICycle::~ICycle() {}

void ICycle::_initParams() {
    setParam<int>("adjust_cycle", 1);              // 调仓周期
    setParam<string>("adjust_mode", "query");      // 调仓模式
    setParam<bool>("delay_to_trading_day", true);  // 延迟至交易日
}

void ICycle::_checkParam(const string& name) const {
    if ("adjust_mode" == name || "adjust_cycle" == name) {
        if (!haveParam("adjust_mode") || !haveParam("adjust_cycle")) {
            // 同时判断两个参数时，可能一个参数还未设定
            return;
        }
        string adjust_mode = getParam<string>("adjust_mode");
        to_lower(adjust_mode);
        int adjust_cycle = getParam<int>("adjust_cycle");
        if ("query" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1);
        } else if ("day" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1);
        } else if ("week" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 5);
        } else if ("month" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 31);
        } else if ("quarter" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 92);
        } else if ("year" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 366);
        } else {
            HKU_THROW("Invalid adjust_mode: {}!", adjust_mode);
        }
    }
}

void ICycle::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
}

Indicator HKU_API CYCLE() {
    return make_shared<ICycle>()->calculate();
}

Indicator HKU_API CYCLE(const KData& k) {
    return Indicator(make_shared<ICycle>(k));
}

} /* namespace hku */
