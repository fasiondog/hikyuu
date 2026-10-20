/*
 * TruncNormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include <algorithm>
#include <cmath>
#include <optional>
#include <random>
#include "TruncNormalSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TruncNormalSlippage)
#endif

namespace hku {

namespace {
// Standard normal CDF via the error function
double normCdf(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

// Inverse standard normal CDF (Acklam's rational approximation)
double normInvCdf(double p) {
    const double lo = 1e-12, hi = 1.0 - 1e-12;
    if (p < lo) {
        p = lo;
    } else if (p > hi) {
        p = hi;
    }
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02,
                               -2.759285104469687e+02, 1.383577518672690e+02,
                               -3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,
                               -1.556989798598866e+02, 6.680131188771972e+01,
                               -1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03, -3.223964580411365e-01,
                               -2.400758277161838e+00, -2.549732539343734e+00,
                               4.374664141464968e+00,  2.938163982698783e+00};
    static const double d[] = {7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00,
                               3.754408661907416e+00};
    double q, r;
    if (p < 0.02425) {
        // Lower tail
        q = std::sqrt(-2.0 * std::log(p));
        return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
               ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }
    if (p <= 0.97575) {
        // Central region
        q = p - 0.5;
        r = q * q;
        return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q /
               (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
    }
    // Upper tail
    q = std::sqrt(-2.0 * std::log(1.0 - p));
    return -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
           ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
}

// Draw a truncated-normal sample in [min_v, max_v] via inverse-CDF (O(1))
double sampleTruncNormal(double mean, double stddev, double min_v, double max_v,
                         std::mt19937& gen) {
    if (stddev <= 0.0) {
        return (min_v + max_v) / 2.0;
    }
    double a = (min_v - mean) / stddev;
    double b = (max_v - mean) / stddev;
    double pa = normCdf(a);
    double pb = normCdf(b);
    double u = std::uniform_real_distribution<double>(0.0, 1.0)(gen);
    double z;
    if (pb - pa < 1e-12) {
        // The truncation interval is a tiny sliver relative to the density: the
        // conditional density is nearly uniform across it
        z = a + u * (b - a);
    } else {
        z = normInvCdf(pa + u * (pb - pa));
    }
    double value = mean + stddev * z;
    // Guard the sampled value against the inverse-CDF approximation error so the
    // truncation bound stays exact
    return std::clamp(value, min_v, max_v);
}
}  // namespace

TruncNormalSlippage::TruncNormalSlippage()
: SlippageBase("SP_TruncNormal"), m_gen(std::random_device{}()) {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
    setParam<double>("min_value", -0.1);
    setParam<double>("max_value", 0.1);
    setParam<double>("short_mean", 0.0);
    setParam<double>("short_stddev", 0.05);
    setParam<double>("short_min_value", -0.1);
    setParam<double>("short_max_value", 0.1);

    // true: unfavorable only; false: symmetric noise
    setParam<bool>("adverse_only", true);

    // 0 means a random seed; a non-zero seed makes the backtest reproducible
    setParam<int64_t>("seed", 0);
}

TruncNormalSlippage::~TruncNormalSlippage() {}

void TruncNormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name || "short_mean" == name) {
        double mean = getParam<double>(name);
        HKU_CHECK(!std::isnan(mean), "SP_TruncNormal: param {} must not be NaN", name);
    } else if ("stddev" == name || "short_stddev" == name) {
        double stddev = getParam<double>(name);
        HKU_CHECK(stddev >= 0.0, "SP_TruncNormal: param {} must be >= 0, got {}", name, stddev);
    } else if ("min_value" == name || "max_value" == name || "short_min_value" == name ||
               "short_max_value" == name) {
        HKU_CHECK(!std::isnan(getParam<double>(name)), "SP_TruncNormal: param {} must not be NaN",
                  name);
    }

    if ("min_value" == name || "max_value" == name) {
        _checkRange("min_value", "max_value");
    } else if ("short_min_value" == name || "short_max_value" == name) {
        _checkRange("short_min_value", "short_max_value");
    }
}

void TruncNormalSlippage::_checkRange(const string& min_name, const string& max_name) const {
    // both bounds of the pair must already be registered; they are not during construction
    if (!haveParam(min_name) || !haveParam(max_name)) {
        return;
    }
    double min_v = getParam<double>(min_name);
    double max_v = getParam<double>(max_name);
    HKU_CHECK(min_v <= max_v, "SP_TruncNormal: param {} must be <= {}, got {} > {}", min_name,
              max_name, min_v, max_v);
    HKU_WARN_IF(std::max(std::abs(min_v), std::abs(max_v)) > 1.0,
                "The amplitude of SP_TruncNormal is too large (>{:.2f}), the slippage may "
                "move the price by more than 100%",
                1.0);
}

void TruncNormalSlippage::_reset() {
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

price_t TruncNormalSlippage::_getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    bool adverse_only = getParam<bool>("adverse_only");

    double value = sampleTruncNormal(mean, stddev, min_v, max_v, m_gen);
    return adverse_only ? price + std::abs(value) : price + value;
}

price_t TruncNormalSlippage::_getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    bool adverse_only = getParam<bool>("adverse_only");

    double value = sampleTruncNormal(mean, stddev, min_v, max_v, m_gen);
    return adverse_only ? price - std::abs(value) : price - value;
}

void TruncNormalSlippage::_calculate() {}

price_t TruncNormalSlippage::_getRealBuyShortPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("short_mean");
    double stddev = getParam<double>("short_stddev");
    double min_v = getParam<double>("short_min_value");
    double max_v = getParam<double>("short_max_value");
    bool adverse_only = getParam<bool>("adverse_only");

    double value = sampleTruncNormal(mean, stddev, min_v, max_v, m_gen);
    return adverse_only ? price + std::abs(value) : price + value;
}

price_t TruncNormalSlippage::_getRealSellShortPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("short_mean");
    double stddev = getParam<double>("short_stddev");
    double min_v = getParam<double>("short_min_value");
    double max_v = getParam<double>("short_max_value");
    bool adverse_only = getParam<bool>("adverse_only");

    double value = sampleTruncNormal(mean, stddev, min_v, max_v, m_gen);
    return adverse_only ? price - std::abs(value) : price - value;
}

SlippagePtr HKU_API SP_TruncNormal(double mean, double stddev, double min_value, double max_value,
                                   bool adverse_only, std::optional<double> short_mean,
                                   std::optional<double> short_stddev,
                                   std::optional<double> short_min_value,
                                   std::optional<double> short_max_value) {
    SlippagePtr ptr = make_shared<TruncNormalSlippage>();
    // Assign the whole set at once so each pair is checked on its final state: setting a single
    // bound would be compared against the other bound's class default mid-way
    Parameter param = ptr->getParameter();
    param.set<double>("mean", mean);
    param.set<double>("stddev", stddev);
    param.set<double>("min_value", min_value);
    param.set<double>("max_value", max_value);
    param.set<bool>("adverse_only", adverse_only);
    param.set<double>("short_mean", short_mean.value_or(mean));
    param.set<double>("short_stddev", short_stddev.value_or(stddev));
    param.set<double>("short_min_value", short_min_value.value_or(min_value));
    param.set<double>("short_max_value", short_max_value.value_or(max_value));
    ptr->setParameter(param);
    return ptr;
}

} /* namespace hku */
