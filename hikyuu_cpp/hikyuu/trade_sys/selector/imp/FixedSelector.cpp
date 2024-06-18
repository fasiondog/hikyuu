/*
 * FixedSelector.cpp
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#include "FixedSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedSelector)
#endif

namespace hku {

FixedSelector::FixedSelector() : SelectorBase("SE_Fixed") {
    setParam<double>("weight", 1.0);
}

FixedSelector::FixedSelector(double weight) : SelectorBase("SE_Fixed") {
    setParam<double>("weight", weight);
}

FixedSelector::~FixedSelector() {}

bool FixedSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SystemWeightList FixedSelector::getSelected(Datetime date) {
    auto weight = getParam<double>("weight");
    SystemWeightList result;
    for (auto& sys : m_real_sys_list) {
        result.emplace_back(sys, weight);
    }
    return result;
}

void FixedSelector::_calculate() {}

SelectorPtr HKU_API SE_Fixed(double weight) {
    return make_shared<FixedSelector>(weight);
}

SelectorPtr HKU_API SE_Fixed(const StockList& stock_list, const SystemPtr& sys, double weight) {
    SelectorPtr p = make_shared<FixedSelector>(weight);
    p->addStockList(stock_list, sys);
    return p;
}

} /* namespace hku */
