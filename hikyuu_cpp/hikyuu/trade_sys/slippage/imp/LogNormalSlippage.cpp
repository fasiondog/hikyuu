/*
 * LogNormalSlippage.cpp
 *
 *  Created on: 2025年10月25日
 *      Author: fasiondog
 */

#include <random>
#include "LogNormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::LogNormalSlippage)
#endif

namespace hku {

std::random_device LogNormalSlippage::ms_rd;
std::mt19937 LogNormalSlippage::ms_gen(ms_rd());

LogNormalSlippage::LogNormalSlippage() : SlippageBase("SP_LogNormal") {
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

    double value = dis(ms_gen);
    // 为了使滑点值在均值附近分布，我们减去exp(mean+stddev^2/2)得到中心化的值
    double centered_value = value - std::exp(mean + stddev * stddev / 2.0);
    // 买入时价格总是变高（不利方向）
    return price + std::abs(centered_value);
}

price_t LogNormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::lognormal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
    // 为了使滑点值在均值附近分布，我们减去exp(mean+stddev^2/2)得到中心化的值
    double centered_value = value - std::exp(mean + stddev * stddev / 2.0);
    // 卖出时价格总是变低（不利方向）
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