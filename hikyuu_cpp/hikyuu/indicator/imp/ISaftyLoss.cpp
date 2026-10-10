/*
 * ISaftyLoss.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "../crt/CVAL.h"
#include "ISaftyLoss.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISaftyLoss)
#endif

namespace hku {

ISaftyLoss::ISaftyLoss() : IndicatorImp("SAFTYLOSS", 1) {
    setParam<int>("n1", 10);
    setParam<int>("n2", 3);
    setParam<double>("p", 2.0);
}

ISaftyLoss::~ISaftyLoss() {}

void ISaftyLoss::_checkParam(const string& name) const {
    if ("n1" == name) {
        HKU_ASSERT(getParam<int>("n1") >= 2);
    } else if ("n2" == name) {
        HKU_ASSERT(getParam<int>("n2") >= 1);
    }
}

void ISaftyLoss::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());
    _readyBuffer(total, 1);

    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");

    m_discard = data.discard() + n1 + n2 - 2;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    double p = getParam<double>("p");
    auto const* src = data.data();
    auto* dst = this->data();
    global_parallel_for_index_void(
      m_discard, total, [&](size_t i) { dst[i] = _calcOneBar(src, i, n1, n2, p); },
      MIN_PARALLEL_WORK / std::max(1, n1 * n2));
}

size_t ISaftyLoss::min_increment_start() const {
    // The inner loop reads src[k - 1] from j == start_pos + 1 - n2 with k starting at j + 2 - n1.
    return getParam<int>("n1") + getParam<int>("n2") - 2;
}

Indicator::value_t ISaftyLoss::_calcOneBar(const Indicator::value_t* src, size_t i, int n1, int n2,
                                           double p) {
    value_t result = 0.0;
    bool has_valid = false;
    for (size_t j = i + 1 - n2; j <= i; ++j) {
        value_t sum = 0.0;
        size_t num = 0;
        for (size_t k = j + 2 - n1; k <= j; ++k) {
            value_t pre = src[k - 1];
            value_t cur = src[k];
            if (pre > cur) {
                sum += pre - cur;
                ++num;
            }
        }

        value_t temp = src[j];
        if (num != 0) {
            temp = temp - (p * sum / num);
        }

        if (std::isnan(temp)) {
            continue;
        }

        if (!has_valid || temp > result) {
            result = temp;
            has_valid = true;
        }
    }

    // A zero line would claim that no stop loss is needed
    return has_valid ? result : Null<value_t>();
}

void ISaftyLoss::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();

    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    double p = getParam<double>("p");

    auto const* src = data.data();
    auto* dst = this->data();

    for (size_t i = start_pos; i < total; ++i) {
        dst[i] = _calcOneBar(src, i, n1, n2, p);
    }
}

void ISaftyLoss::_dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2, double p) {
    HKU_IF_RETURN(n1 < 2 || n2 < 1, void());
    // Intentional: the result at curPos only depends on the trailing [curPos + 2 - n1 - n2, curPos]
    // window, so compute it directly from ind instead of rebuilding the whole prefix. The value is
    // bitwise identical to SAFTYLOSS(SLICE(ind, 0, curPos + 1)) and drops from O(curPos) to
    // O(n1 * n2) per bar. Before the static discard (n1 + n2 - 2 bars after ind.discard()), the
    // old slice-based path yielded Null, which is preserved here.
    size_t start = ind.discard();
    if (curPos + 1 <= start + n1 + n2 - 2) {
        _set(Null<price_t>(), curPos);
        return;
    }

    _set(_calcOneBar(ind.data(), curPos, n1, n2, p), curPos);
}

void ISaftyLoss::_dyn_calculate(const Indicator& ind) {
    auto iter = m_ind_params.find("n1");
    Indicator n1 =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n1"));
    iter = m_ind_params.find("n2");
    Indicator n2 =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n2"));
    iter = m_ind_params.find("p");
    Indicator p =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<double>("p"));

    HKU_CHECK(n1.size() == ind.size(), "ind_param(n1).size()={}, ind.size()={}!", n1.size(),
              ind.size());
    HKU_CHECK(n2.size() == ind.size(), "ind_param(n2).size()={}, ind.size()={}!", n2.size(),
              ind.size());
    HKU_CHECK(p.size() == ind.size(), "ind_param(p).size()={}, ind.size()={}!", p.size(),
              ind.size());

    m_discard = std::max(ind.discard(), n1.discard());
    m_discard = std::max(m_discard, n2.discard());
    m_discard = std::max(m_discard, p.discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    global_parallel_for_index_void(
      ind.discard(), total, [&](size_t i) { _dyn_one_circle(ind, i, n1[i], n2[i], p[i]); }, 400);

    updateDiscard();
}

Indicator HKU_API SAFTYLOSS(int n1, int n2, double p) {
    IndicatorImpPtr result = make_shared<ISaftyLoss>();
    result->setParam<int>("n1", n1);
    result->setParam<int>("n2", n2);
    result->setParam<double>("p", p);
    return Indicator(result);
}

Indicator HKU_API SAFTYLOSS(const IndParam& n1, const IndParam& n2, double p) {
    IndicatorImpPtr result = make_shared<ISaftyLoss>();
    result->setIndParam("n1", n1);
    result->setIndParam("n2", n2);
    result->setParam<double>("p", p);
    return Indicator(result);
}

Indicator HKU_API SAFTYLOSS(const IndParam& n1, const IndParam& n2, const IndParam& p) {
    IndicatorImpPtr result = make_shared<ISaftyLoss>();
    result->setIndParam("n1", n1);
    result->setIndParam("n2", n2);
    result->setIndParam("p", p);
    return Indicator(result);
}

} /* namespace hku */
