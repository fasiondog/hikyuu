/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "hikyuu/trade_manage/Performance.h"
#include "OptimalSelector.h"

namespace hku {

OptimalSelector::OptimalSelector() : SelectorBase("SE_Optimal") {
    setParam<bool>("depend_on_proto_sys", true);
    setParam<string>("key", "帐户平均年收益率%");
    setParam<int>("mode", 0);
}

void OptimalSelector::_checkParam(const string& name) const {
    if ("mode" == name) {
        int mode = getParam<int>(name);
        HKU_ASSERT(0 == mode || 1 == mode);
    } else if ("key" == name) {
        string key = getParam<string>("key");
        HKU_CHECK(Performance::exist(key), R"(Invalid key("{}") in Performance!)", key);
    }
}

SystemWeightList OptimalSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    return ret;
}

bool OptimalSelector::isMatchAF(const AFPtr& af) {
    return true;
}

void OptimalSelector::_calculate() {}

}  // namespace hku