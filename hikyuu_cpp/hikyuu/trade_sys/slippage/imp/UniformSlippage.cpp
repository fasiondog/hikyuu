/*
 * UniformSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#include <algorithm>
#include <cmath>
#include <optional>

#include "UniformSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::UniformSlippage)
#endif

namespace hku {

UniformSlippage::UniformSlippage() : SlippageBase("SP_Uniform"), m_gen(std::random_device{}()) {
    setParam<double>("min_value", -0.05);
    setParam<double>("max_value", 0.05);
    setParam<double>("short_min_value", -0.05);
    setParam<double>("short_max_value", 0.05);

    // true: unfavorable only; false: symmetric noise
    setParam<bool>("adverse_only", true);

    // 0 means a random seed; a non-zero seed makes the backtest reproducible
    setParam<int64_t>("seed", 0);
}

UniformSlippage::~UniformSlippage() {}

void UniformSlippage::_checkParam(const string& name) const {
    if ("min_value" == name || "max_value" == name || "short_min_value" == name ||
        "short_max_value" == name) {
        HKU_CHECK(!std::isnan(getParam<double>(name)), "SP_Uniform: param {} must not be NaN",
                  name);
    }

    if ("min_value" == name || "max_value" == name) {
        _checkRange("min_value", "max_value");
    } else if ("short_min_value" == name || "short_max_value" == name) {
        _checkRange("short_min_value", "short_max_value");
    }
}

void UniformSlippage::_checkRange(const string& min_name, const string& max_name) const {
    // both bounds of the pair must already be registered; they are not during construction
    if (!haveParam(min_name) || !haveParam(max_name)) {
        return;
    }
    double min_v = getParam<double>(min_name);
    double max_v = getParam<double>(max_name);
    HKU_CHECK(min_v <= max_v, "SP_Uniform: param {} must be <= {}, got {} > {}", min_name, max_name,
              min_v, max_v);
    HKU_WARN_IF(std::max(std::abs(min_v), std::abs(max_v)) > 1.0,
                "The amplitude of SP_Uniform is too large (>{:.2f}), the slippage may "
                "move the price by more than 100%",
                1.0);
}

void UniformSlippage::_reset() {
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

price_t UniformSlippage ::_getRealBuyPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    bool adverse_only = getParam<bool>("adverse_only");
    std::uniform_real_distribution<double> dis(min_v, max_v);
    double value = dis(m_gen);
    return adverse_only ? price + std::abs(value) : price + value;
}

price_t UniformSlippage ::_getRealSellPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    bool adverse_only = getParam<bool>("adverse_only");
    std::uniform_real_distribution<double> dis(min_v, max_v);
    double value = dis(m_gen);
    return adverse_only ? price - std::abs(value) : price - value;
}

void UniformSlippage::_calculate() {}

price_t UniformSlippage::_getRealBuyShortPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("short_min_value");
    double max_v = getParam<double>("short_max_value");
    bool adverse_only = getParam<bool>("adverse_only");
    std::uniform_real_distribution<double> dis(min_v, max_v);
    double value = dis(m_gen);
    return adverse_only ? price + std::abs(value) : price + value;
}

price_t UniformSlippage::_getRealSellShortPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("short_min_value");
    double max_v = getParam<double>("short_max_value");
    bool adverse_only = getParam<bool>("adverse_only");
    std::uniform_real_distribution<double> dis(min_v, max_v);
    double value = dis(m_gen);
    return adverse_only ? price - std::abs(value) : price - value;
}

SlippagePtr HKU_API SP_Uniform(double min_value, double max_value, bool adverse_only,
                               std::optional<double> short_min_value,
                               std::optional<double> short_max_value) {
    SlippagePtr ptr = make_shared<UniformSlippage>();
    // Assign the whole set at once so each pair is checked on its final state: setting a single
    // bound would be compared against the other bound's class default mid-way
    Parameter param = ptr->getParameter();
    param.set<double>("min_value", min_value);
    param.set<double>("max_value", max_value);
    param.set<bool>("adverse_only", adverse_only);
    param.set<double>("short_min_value", short_min_value.value_or(min_value));
    param.set<double>("short_max_value", short_max_value.value_or(max_value));
    ptr->setParameter(param);
    return ptr;
}

} /* namespace hku */
