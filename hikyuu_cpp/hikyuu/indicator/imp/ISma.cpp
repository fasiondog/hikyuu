/*
 * ISma.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#include "../crt/SLICE.h"
#include "../crt/SMA.h"
#include "../crt/CVAL.h"
#include "ISma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISma)
#endif

namespace hku {

ISma::ISma() : IndicatorImp("SMA", 1) {
    setParam<int>("n", 22);
    setParam<double>("m", 2.0);
}

ISma::~ISma() {}

bool ISma::check() {
    return getParam<int>("n") >= 1;
}

void ISma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    double n = getParam<int>("n");
    double m = getParam<double>("m");

    auto const* src = ind.data();
    auto* dst = this->data();

    double p = n - m;
    dst[m_discard] = src[m_discard];
    for (size_t i = m_discard + 1; i < total; i++) {
        dst[i] = (m * src[i] + p * dst[i - 1]) / n;
    }
}

void ISma::_dyn_one_circle(const Indicator& ind, size_t curPos, int n, double m) {
    HKU_IF_RETURN(n < 1, void());
    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator sma = SMA(slice, n, m);
    if (sma.size() > 0) {
        _set(sma[sma.size() - 1], curPos);
    }
}

void ISma::_dyn_calculate(const Indicator& ind) {
    auto iter = m_ind_params.find("n");
    Indicator n =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n"));
    iter = m_ind_params.find("m");
    Indicator m =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("m"));

    HKU_CHECK(n.size() == ind.size(), "ind_param(n).size()={}, ind.size()={}!", n.size(),
              ind.size());
    HKU_CHECK(m.size() == ind.size(), "ind_param(m).size()={}, ind.size()={}!", m.size(),
              ind.size());

    m_discard = std::max(ind.discard(), n.discard());
    m_discard = std::max(m_discard, m.discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    static const size_t minCircleLength = 400;
    size_t workerNum = ms_tg->worker_num();
    if (total < minCircleLength || workerNum == 1) {
        for (size_t i = ind.discard(); i < total; i++) {
            _dyn_one_circle(ind, i, n[i], m[i]);
        }
        _update_discard();
        return;
    }

    size_t circleLength = minCircleLength;
    if (minCircleLength * workerNum < total) {
        size_t tailCount = total % workerNum;
        circleLength = tailCount == 0 ? total / workerNum : total / workerNum + 1;
    }

    std::vector<std::future<void>> tasks;
    for (size_t group = 0; group < workerNum; group++) {
        size_t first = circleLength * group;
        if (first >= total) {
            break;
        }
        tasks.push_back(ms_tg->submit([=, &ind, &n, &m]() {
            size_t endPos = first + circleLength;
            if (endPos > total) {
                endPos = total;
            }
            for (size_t i = circleLength * group; i < endPos; i++) {
                _dyn_one_circle(ind, i, n[i], m[i]);
            }
        }));
    }

    for (auto& task : tasks) {
        task.get();
    }
    _update_discard();
}

Indicator HKU_API SMA(int n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

Indicator HKU_API SMA(int n, const IndParam& m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setIndParam("m", m);
    return Indicator(p);
}

Indicator HKU_API SMA(const IndParam& n, const IndParam& m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setIndParam("n", n);
    p->setIndParam("m", m);
    return Indicator(p);
}

Indicator HKU_API SMA(const IndParam& n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setIndParam("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

} /* namespace hku */
