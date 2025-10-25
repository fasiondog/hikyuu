/*
 * TruncNormalSlippage.cpp
 *
 *  Created on: 2025年10月25日
 *      Author: fasiondog
 */

#include <random>
#include "TruncNormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TruncNormalSlippage)
#endif

namespace hku {

std::random_device TruncNormalSlippage::ms_rd;
std::mt19937 TruncNormalSlippage::ms_gen(ms_rd());

TruncNormalSlippage::TruncNormalSlippage() : SlippageBase("SP_TruncNormal") {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
    setParam<double>("min_value", -0.1);
    setParam<double>("max_value", 0.1);
}

TruncNormalSlippage::~TruncNormalSlippage() {}

void TruncNormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name) {
        HKU_ASSERT(!std::isnan(getParam<double>("mean")));
    } else if ("stddev" == name) {
        HKU_ASSERT(getParam<double>("stddev") >= 0.0);
    } else if ("min_value" == name) {
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

price_t TruncNormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");

    std::normal_distribution<double> dis(mean, stddev);

    double value;
    // 生成符合截断范围的值
    do {
        value = dis(ms_gen);
    } while (value < min_v || value > max_v);

    // 买入时价格总是变高（不利方向）
    return price + std::abs(value);
}

price_t TruncNormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");

    std::normal_distribution<double> dis(mean, stddev);

    double value;
    // 生成符合截断范围的值
    do {
        value = dis(ms_gen);
    } while (value < min_v || value > max_v);

    // 卖出时价格总是变低（不利方向）
    return price - std::abs(value);
}

void TruncNormalSlippage::_calculate() {}

SlippagePtr HKU_API SP_TruncNormal(double mean, double stddev, double min_value, double max_value) {
    SlippagePtr ptr = make_shared<TruncNormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    ptr->setParam("min_value", min_value);
    ptr->setParam("max_value", max_value);
    return ptr;
}

} /* namespace hku */