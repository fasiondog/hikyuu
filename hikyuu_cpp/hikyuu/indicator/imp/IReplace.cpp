/*
 * IReplace.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#include "IReplace.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IReplace)
#endif

namespace hku {

namespace {

/**
 * Compare two values within a relative tolerance of a few ULPs. An absolute epsilon (e.g.
 * numeric_limits::epsilon()) is scale-sensitive: it misses equal-by-intent values at price
 * magnitudes (ULP of 100.0 is ~1.4e-14) while falsely matching tiny non-zero values near 0
 */
inline bool nearly_equal(Indicator::value_t a, Indicator::value_t b) {
    if (a == b)
        return true;  // bitwise equal, including +0.0 == -0.0
    // NaN never matches; infinities match only via ==, otherwise the tolerance band becomes inf
    if (std::isnan(a) || std::isnan(b) || std::isinf(a) || std::isinf(b))
        return false;
    Indicator::value_t scale = std::max(std::fabs(a), std::fabs(b));
    return std::fabs(a - b) <= 8 * std::numeric_limits<Indicator::value_t>::epsilon() * scale;
}

}  // namespace

IReplace::IReplace() : IndicatorImp("REPLACE", 1) {
    setParam<double>("old_value", Null<double>());
    setParam<double>("new_value", 0.0);
    setParam<bool>("ignore_discard",
                   false);  // Ignore the discard of the passed indicator, i.e. replace all the data
}

IReplace::~IReplace() {}

void IReplace::_calculate(const Indicator &data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    bool ignore_discard = getParam<bool>("ignore_discard");
    if (ignore_discard) {
        m_discard = 0;
    } else {
        m_discard = data.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
    }

    value_t old_value = getParam<double>("old_value");
    value_t new_value = getParam<double>("new_value");

    auto const *src = data.data();
    auto *dst = this->data();

    if (std::isnan(old_value)) {
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = std::isnan(src[i]) ? new_value : src[i];
        }
    } else {
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = nearly_equal(src[i], old_value) ? new_value : src[i];
        }
    }

    // Update m_discard again
    updateDiscard();
}

Indicator HKU_API REPLACE(double old_value, double new_value, bool ignore_discard) {
    HKU_WARN_IF(nearly_equal(old_value, new_value),
                "The value to be replaced is equal to the replacement value! Are you sure?");
    auto p = make_shared<IReplace>();
    p->setParam<double>("old_value", old_value);
    p->setParam<double>("new_value", new_value);
    p->setParam<bool>("ignore_discard", ignore_discard);
    return Indicator(p);
}

} /* namespace hku */
