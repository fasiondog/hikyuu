/*
 * IReplace.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "IReplace.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IReplace)
#endif

namespace hku {

IReplace::IReplace() : IndicatorImp("REPLACE", 1) {
    setParam<double>("old_value", Null<double>());
    setParam<double>("new_value", 0.0);
    setParam<bool>("ignore_discard",
                   false);  // 忽略传入指标的 discard, 如果替换nan值，新结果 discard 将为 0
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
        value_t epsilon = std::numeric_limits<value_t>::epsilon();
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = (std::fabs(src[i] - old_value) < epsilon) ? new_value : src[i];
        }
    }

    // 重新更新 m_discard
    for (size_t i = m_discard; i < total; ++i) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HKU_API REPLACE(double old_value, double new_value, bool ignore_discard) {
    Indicator::value_t epsilon = std::numeric_limits<Indicator::value_t>::epsilon();
    HKU_WARN_IF(std::fabs(old_value - new_value) < epsilon,
                "The value to be replaced is equal to the replacement value! Are you sure?");
    auto p = make_shared<IReplace>();
    p->setParam<double>("old_value", old_value);
    p->setParam<double>("new_value", new_value);
    p->setParam<bool>("ignore_discard", ignore_discard);
    return Indicator(p);
}

} /* namespace hku */
