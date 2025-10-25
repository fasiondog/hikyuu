/*
 * NormalSlippage.cpp
 *
 *  Created on: 2025年10月25日
 *      Author: fasiondog
 */

#include <random>
#include "NormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormalSlippage)
#endif

namespace hku {

std::random_device NormalSlippage::ms_rd;
std::mt19937 NormalSlippage::ms_gen(ms_rd());

NormalSlippage::NormalSlippage() : SlippageBase("SP_Normal") {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
}

NormalSlippage::~NormalSlippage() {}

void NormalSlippage::_checkParam(const string& name) const {
    if ("stddev" == name) {
        HKU_ASSERT(getParam<double>("stddev") >= 0.0);
    }
}

price_t NormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
    return price + std::abs(value);
}

price_t NormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
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