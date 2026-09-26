/*
 * UniformSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#include "UniformSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::UniformSlippage)
#endif

namespace hku {

UniformSlippage::UniformSlippage() : SlippageBase("SP_Uniform"), m_gen(std::random_device{}()) {
    setParam<double>("min_value", -0.05);
    setParam<double>("max_value", 0.05);
}

UniformSlippage::~UniformSlippage() {}

void UniformSlippage::_checkParam(const string& name) const {
    if ("min_value" == name) {
        HKU_ASSERT(!std::isnan(getParam<double>("min_value")));
    } else if ("max_value" == name) {
        HKU_ASSERT(!std::isnan(getParam<double>("max_value")));
    }

    if (haveParam("min_value") && haveParam("max_value")) {
        double min_v = getParam<double>("min_value");
        double max_v = getParam<double>("max_value");
        HKU_ASSERT(min_v <= max_v);
    }
}

price_t UniformSlippage ::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    std::uniform_real_distribution<double> dis(min_v, max_v);
    double value = dis(m_gen);
    return price + std::abs(value);
}

price_t UniformSlippage ::getRealSellPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    std::uniform_real_distribution<double> dis(min_v, max_v);
    double value = dis(m_gen);
    return price - std::abs(value);
}

void UniformSlippage::_calculate() {}

SlippagePtr HKU_API SP_Uniform(double min_value, double max_value) {
    SlippagePtr ptr = make_shared<UniformSlippage>();
    ptr->setParam("min_value", min_value);
    ptr->setParam("max_value", max_value);
    return ptr;
}

} /* namespace hku */
