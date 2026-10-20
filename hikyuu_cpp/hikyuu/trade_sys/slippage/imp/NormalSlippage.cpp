/*
 * NormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include <cmath>
#include <optional>
#include <random>
#include "NormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormalSlippage)
#endif

namespace hku {

NormalSlippage::NormalSlippage() : SlippageBase("SP_Normal"), m_gen(std::random_device{}()) {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
    setParam<double>("short_mean", 0.0);
    setParam<double>("short_stddev", 0.05);

    // true: unfavorable only; false: symmetric noise
    setParam<bool>("adverse_only", true);

    // 0 means a random seed; a non-zero seed makes the backtest reproducible
    setParam<int64_t>("seed", 0);
}

NormalSlippage::~NormalSlippage() {}

void NormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name || "short_mean" == name) {
        double mean = getParam<double>(name);
        HKU_CHECK(!std::isnan(mean), "SP_Normal: param {} must not be NaN", name);
    } else if ("stddev" == name || "short_stddev" == name) {
        double stddev = getParam<double>(name);
        HKU_CHECK(stddev >= 0.0, "SP_Normal: param {} must be >= 0, got {}", name, stddev);
        HKU_WARN_IF(stddev > 1.0,
                    "The stddev of SP_Normal is too large (>{:.2f}); the slippage may be extreme",
                    1.0);
    }
}

void NormalSlippage::_reset() {
    int64_t seed = getParam<int64_t>("seed");
    if (seed != 0) {
        uint64_t useed = static_cast<uint64_t>(seed);
        if (useed > 0xffffffffull) {
            // A seed beyond the 32-bit range would silently lose its high bits on truncation:
            // mix the full 64 bits instead (seeds within the 32-bit range keep the historical
            // seeding so the existing reproducible sequences stay unchanged)
            std::seed_seq seq{static_cast<uint32_t>(useed), static_cast<uint32_t>(useed >> 32)};
            m_gen.seed(seq);
        } else {
            m_gen.seed(static_cast<uint32_t>(useed));
        }
    } else {
        m_gen.seed(std::random_device{}());
    }
}

price_t NormalSlippage::_getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    bool adverse_only = getParam<bool>("adverse_only");

    // A zero stddev is legal in the parameter domain but is outside the standard precondition of
    // normal_distribution (0 < stddev): pin the draw to the mean instead of relying on what the
    // library does with a degenerate deviation
    double value = mean;
    if (stddev > 0.0) {
        std::normal_distribution<double> dis(mean, stddev);
        value = dis(m_gen);
    }
    return adverse_only ? price + std::abs(value) : price + value;
}

price_t NormalSlippage::_getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    bool adverse_only = getParam<bool>("adverse_only");

    // A zero stddev is legal in the parameter domain but is outside the standard precondition of
    // normal_distribution (0 < stddev): pin the draw to the mean instead of relying on what the
    // library does with a degenerate deviation
    double value = mean;
    if (stddev > 0.0) {
        std::normal_distribution<double> dis(mean, stddev);
        value = dis(m_gen);
    }
    return adverse_only ? price - std::abs(value) : price - value;
}

void NormalSlippage::_calculate() {}

price_t NormalSlippage::_getRealBuyShortPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("short_mean");
    double stddev = getParam<double>("short_stddev");
    bool adverse_only = getParam<bool>("adverse_only");

    // A zero stddev is legal in the parameter domain but is outside the standard precondition of
    // normal_distribution (0 < stddev): pin the draw to the mean instead of relying on what the
    // library does with a degenerate deviation
    double value = mean;
    if (stddev > 0.0) {
        std::normal_distribution<double> dis(mean, stddev);
        value = dis(m_gen);
    }
    return adverse_only ? price + std::abs(value) : price + value;
}

price_t NormalSlippage::_getRealSellShortPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("short_mean");
    double stddev = getParam<double>("short_stddev");
    bool adverse_only = getParam<bool>("adverse_only");

    // A zero stddev is legal in the parameter domain but is outside the standard precondition of
    // normal_distribution (0 < stddev): pin the draw to the mean instead of relying on what the
    // library does with a degenerate deviation
    double value = mean;
    if (stddev > 0.0) {
        std::normal_distribution<double> dis(mean, stddev);
        value = dis(m_gen);
    }
    return adverse_only ? price - std::abs(value) : price - value;
}

SlippagePtr HKU_API SP_Normal(double mean, double stddev, bool adverse_only,
                              std::optional<double> short_mean,
                              std::optional<double> short_stddev) {
    SlippagePtr ptr = make_shared<NormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    ptr->setParam("adverse_only", adverse_only);
    ptr->setParam("short_mean", short_mean.value_or(mean));
    ptr->setParam("short_stddev", short_stddev.value_or(stddev));
    return ptr;
}

} /* namespace hku */
