/*
 * LogNormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include <random>
#include "LogNormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::LogNormalSlippage)
#endif

namespace hku {

LogNormalSlippage::LogNormalSlippage()
: SlippageBase("SP_LogNormal"), m_gen(std::random_device{}()) {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
}

LogNormalSlippage::~LogNormalSlippage() {}

void LogNormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name) {
        HKU_ASSERT(!std::isnan(getParam<double>("mean")));
    } else if ("stddev" == name) {
        HKU_ASSERT(getParam<double>("stddev") >= 0.0);
    }
}

price_t LogNormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::lognormal_distribution<double> dis(mean, stddev);

    double value = dis(m_gen);
    // To distribute the slippage values around the mean, exp(mean+stddev^2/2) is subtracted to
    // center them
    double centered_value = value - std::exp(mean + stddev * stddev / 2.0);
    // On a buy the price always goes higher (the unfavorable direction)
    return price + std::abs(centered_value);
}

price_t LogNormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::lognormal_distribution<double> dis(mean, stddev);

    double value = dis(m_gen);
    // To distribute the slippage values around the mean, exp(mean+stddev^2/2) is subtracted to
    // center them
    double centered_value = value - std::exp(mean + stddev * stddev / 2.0);
    // On a sell the price always goes lower (the unfavorable direction)
    return price - std::abs(centered_value);
}

void LogNormalSlippage::_calculate() {}

SlippagePtr HKU_API SP_LogNormal(double mean, double stddev) {
    SlippagePtr ptr = make_shared<LogNormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    return ptr;
}

} /* namespace hku */