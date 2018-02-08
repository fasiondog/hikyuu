/*
 * EqualWeightAllocateFunds.cpp
 *
 *  Created on: 2018年2月8日
 *      Author: fasiondog
 */

#include "EqualWeightAllocateFunds.h"

namespace hku {

EqualWeightAllocateFunds::EqualWeightAllocateFunds()
:AllocateFundsBase("AF_EqualWeight") {

}

EqualWeightAllocateFunds::~EqualWeightAllocateFunds() {

}

SystemWeightList EqualWeightAllocateFunds
::_allocateWeight(const SystemList& se_list, const SystemList& hold_list) {
    SystemWeightList result;

    SystemList temp_list;
    if (getParam<bool>("adjust_hold_sys")) {
        copy(se_list.begin(), se_list.end(), temp_list.begin());
        copy(hold_list.begin(), hold_list.end(), temp_list.end());
    } else {
        copy(se_list.begin(), se_list.end(), temp_list.begin());
    }

    size_t total = temp_list.size();
    if (total == 0) {
        return result;
    }

    price_t weight = 100.0 / total;
    for (auto iter = temp_list.begin(); iter != temp_list.end(); ++iter) {
        SystemWeight sw;
        sw.sys = *iter;
        sw.weight = weight;
        result.push_back(sw);
    }

    return result;
}

AFPtr HKU_API AF_EqualWeight() {
    return make_shared<EqualWeightAllocateFunds>();
}

} /* namespace hku */
