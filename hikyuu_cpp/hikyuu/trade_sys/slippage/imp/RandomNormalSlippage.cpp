/*
 * RandomNormalSlippage.cpp
 *
 *  Created on: 2025年10月25日
 *      Author: fasiondog
 */

#include <random>
#include "RandomNormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::RandomNormalSlippage)
#endif

namespace hku {

RandomNormalSlippage::RandomNormalSlippage() : SlippageBase("SP_RandomNormal") {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
}

RandomNormalSlippage::~RandomNormalSlippage() {}

void RandomNormalSlippage::_checkParam(const string& name) const {
    if ("stddev" == name) {
        HKU_ASSERT(getParam<double>("stddev") >= 0.0);
    }
}

price_t RandomNormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    // 使用静态随机数生成器以避免重复初始化开销
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(gen);
    return price + value;
}

price_t RandomNormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    return getRealBuyPrice(datetime, price);
}

void RandomNormalSlippage::_calculate() {}

SlippagePtr HKU_API SP_RandomNormal(double mean, double stddev) {
    SlippagePtr ptr = make_shared<RandomNormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    return ptr;
}

} /* namespace hku */