/*
 * NormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include <random>
#include "NormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormalSlippage)
#endif

namespace hku {

NormalSlippage::NormalSlippage() : SlippageBase("SP_Normal"), m_gen(std::random_device{}()) {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
    // 0 means a random seed; a non-zero seed makes the backtest reproducible
    setParam<int64_t>("seed", 0);
}

NormalSlippage::~NormalSlippage() {}

void NormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name) {
        HKU_ASSERT(!std::isnan(getParam<double>("mean")));
    } else if ("stddev" == name) {
        HKU_ASSERT(getParam<double>("stddev") >= 0.0);
    }
}

void NormalSlippage::_reset() {
    int64_t seed = getParam<int64_t>("seed");
    if (seed != 0) {
        m_gen.seed(static_cast<uint32_t>(seed));
    } else {
        m_gen.seed(std::random_device{}());
    }
}

price_t NormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(m_gen);
    return price + std::abs(value);
}

price_t NormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(m_gen);
    return price - std::abs(value);
}

void NormalSlippage::_calculate() {}

SlippagePtr HKU_API SP_Normal(double mean, double stddev) {
    SlippagePtr ptr = make_shared<NormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    return ptr;
}

} /* namespace hku */