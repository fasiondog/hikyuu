/*
 * IMacd.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "IMacd.h"
#include "../crt/EMA.h"
#include "../crt/MACD.h"
#include "../crt/SLICE.h"
#include "../crt/CVAL.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMacd)
#endif

namespace hku {

IMacd::IMacd() : IndicatorImp("MACD", 3) {
    setParam<int>("n1", 12);
    setParam<int>("n2", 26);
    setParam<int>("n3", 9);
}

IMacd::~IMacd() {}

void IMacd::_checkParam(const string& name) const {
    if ("n1" == name) {
        HKU_ASSERT(getParam<int>("n1") >= 0);
    } else if ("n2" == name) {
        HKU_ASSERT(getParam<int>("n2") >= 0);
    } else if ("n3" == name) {
        HKU_ASSERT(getParam<int>("n3") >= 0);
    }
}

void IMacd::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 3);

    m_discard = data.discard();
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard + 1);
}

size_t IMacd::min_increment_start() const {
    return 1;
}

void IMacd::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    int n3 = getParam<int>("n3");

    auto const* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);

    price_t m1 = 2.0 / (n1 + 1);
    price_t m2 = 2.0 / (n2 + 1);
    price_t m3 = 2.0 / (n3 + 1);
    price_t ema1 = src[start_pos - 1];
    price_t ema2 = src[start_pos - 1];
    price_t diff = 0.0;
    price_t dea = 0.0;
    price_t bar = 0.0;
    dst0[start_pos - 1] = bar;
    dst1[start_pos - 1] = diff;
    dst2[start_pos - 1] = dea;

    size_t total = data.size();
    for (size_t i = start_pos; i < total; ++i) {
        ema1 = (src[i] - ema1) * m1 + ema1;
        ema2 = (src[i] - ema2) * m2 + ema2;
        diff = ema1 - ema2;
        dea = diff * m3 + dea - dea * m3;
        bar = diff - dea;
        dst0[i] = bar;
        dst1[i] = diff;
        dst2[i] = dea;
    }
}

void IMacd::_dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2, int n3) {
    HKU_IF_RETURN(n1 <= 0 || n2 <= 0 || n3 <= 0, void());
    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator macd = MACD(slice, n1, n2, n3);
    if (macd.size() > 0) {
        size_t index = macd.size() - 1;
        _set(macd.get(index, 0), curPos, 0);
        _set(macd.get(index, 1), curPos, 1);
        _set(macd.get(index, 2), curPos, 2);
    }
}

void IMacd::_dyn_calculate(const Indicator& ind) {
    auto iter = m_ind_params.find("n1");
    Indicator n1 =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n1"));
    iter = m_ind_params.find("n2");
    Indicator n2 =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n2"));
    iter = m_ind_params.find("n3");
    Indicator n3 =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n3"));

    HKU_CHECK(n1.size() == ind.size(), "ind_param(n2).size()={}, ind.size()={}!", n2.size(),
              ind.size());
    HKU_CHECK(n2.size() == ind.size(), "ind_param(n2).size()={}, ind.size()={}!", n2.size(),
              ind.size());
    HKU_CHECK(n3.size() == ind.size(), "ind_param(n3).size()={}, ind.size()={}!", n3.size(),
              ind.size());

    m_discard = std::max(ind.discard(), n2.discard());
    m_discard = std::max(m_discard, n3.discard());
    m_discard = std::max(m_discard, n1.discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    global_parallel_for_index_void(ind.discard(), total,
                                   [&](size_t i) { _dyn_one_circle(ind, i, n1[i], n2[i], n3[i]); });

    _update_discard();
}

Indicator HKU_API MACD(int n1, int n2, int n3) {
    IndicatorImpPtr p = make_shared<IMacd>();
    p->setParam<int>("n1", n1);
    p->setParam<int>("n2", n2);
    p->setParam<int>("n3", n3);
    return Indicator(p);
}

Indicator HKU_API MACD(const IndParam& n1, const IndParam& n2, const IndParam& n3) {
    IndicatorImpPtr p = make_shared<IMacd>();
    p->setIndParam("n1", n1);
    p->setIndParam("n2", n2);
    p->setIndParam("n3", n3);
    return Indicator(p);
}

} /* namespace hku */
