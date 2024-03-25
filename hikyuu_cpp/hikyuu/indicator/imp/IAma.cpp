/*
 * IAma.cpp
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#include <cmath>
#include "../crt/AMA.h"
#include "../crt/CVAL.h"
#include "../crt/SLICE.h"
#include "IAma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAma)
#endif

namespace hku {

IAma::IAma() : IndicatorImp("AMA", 2) {
    setParam<int>("n", 10);
    setParam<int>("fast_n", 2);
    setParam<int>("slow_n", 30);
}

IAma::~IAma() {}

void IAma::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1);
    } else if ("fast_n" == name) {
        HKU_ASSERT(getParam<int>("fast_n") >= 0);
    } else if ("slow_n" == name) {
        HKU_ASSERT(getParam<int>("slow_n") >= 0);
    }
}

void IAma::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);

    int n = getParam<int>("n");
    int fast_n = getParam<int>("fast_n");
    int slow_n = getParam<int>("slow_n");

    size_t start = m_discard;

    price_t fastest = 2.0 / (fast_n + 1);
    price_t slowest = 2.0 / (slow_n + 1);
    price_t delta = fastest - slowest;

    price_t prevol = 0.0, vol = 0.0, er = 1.0;
    price_t ama = src[start];
    size_t first_end = start + n + 1 >= total ? total : start + n + 1;
    _set(ama, start, 0);
    _set(er, start, 1);
    for (size_t i = start + 1; i < first_end; ++i) {
        vol += std::fabs(src[i] - src[i - 1]);
        er = (vol == 0.0) ? 1.0 : (src[i] - src[start]) / vol;
        if (er > 1.0)
            er = 1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
        dst0[i] = ama;
        dst1[i] = er;
    }

    prevol = vol;
    for (size_t i = first_end; i < total; ++i) {
        vol = prevol + std::fabs(src[i] - src[i - 1]) - std::fabs(src[i + 1 - n] - src[i - n]);
        er = (vol == 0.0) ? 1.0 : (src[i] - src[i - n]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
        prevol = vol;
        dst0[i] = ama;
        dst1[i] = er;
    }
}

void IAma::_dyn_one_circle(const Indicator& ind, size_t curPos, int n, int fast_n, int slow_n) {
    if (n < 1) {
        n = 1;
    }

    if (fast_n < 0) {
        fast_n = 0;
    }

    if (slow_n < 0) {
        slow_n = 0;
    }

    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator ama = AMA(slice, n, fast_n, slow_n);
    if (ama.size() > 0) {
        size_t index = ama.size() - 1;
        _set(ama.get(index, 0), curPos, 0);
        _set(ama.get(index, 1), curPos, 1);
    }
}

void IAma::_dyn_calculate(const Indicator& ind) {
    auto iter = m_ind_params.find("fast_n");
    Indicator fast_n =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("fast_n"));
    iter = m_ind_params.find("slow_n");
    Indicator slow_n =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("slow_n"));
    iter = m_ind_params.find("n");
    Indicator n =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n"));

    HKU_CHECK(fast_n.size() == ind.size(), "ind_param(fast_n).size()={}, ind.size()={}!",
              fast_n.size(), ind.size());
    HKU_CHECK(slow_n.size() == ind.size(), "ind_param(slow_n).size()={}, ind.size()={}!",
              slow_n.size(), ind.size());

    m_discard = std::max(ind.discard(), fast_n.discard());
    m_discard = std::max(m_discard, slow_n.discard());
    m_discard = std::max(m_discard, n.discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    static const size_t minCircleLength = 400;
    size_t workerNum = ms_tg->worker_num();
    if (total < minCircleLength || workerNum == 1) {
        for (size_t i = ind.discard(); i < total; i++) {
            _dyn_one_circle(ind, i, n[i], fast_n[i], slow_n[i]);
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
        tasks.push_back(ms_tg->submit([=, &ind, &n, &fast_n, &slow_n]() {
            size_t endPos = first + circleLength;
            if (endPos > total) {
                endPos = total;
            }
            for (size_t i = circleLength * group; i < endPos; i++) {
                _dyn_one_circle(ind, i, n[i], fast_n[i], slow_n[i]);
            }
        }));
    }

    for (auto& task : tasks) {
        task.get();
    }
    _update_discard();
}

Indicator HKU_API AMA(int n, int fast_n, int slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setParam<int>("n", n);
    p->setParam<int>("fast_n", fast_n);
    p->setParam<int>("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(int n, const IndParam& fast_n, int slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setParam<int>("n", n);
    p->setIndParam("fast_n", fast_n);
    p->setParam<int>("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(int n, const IndParam& fast_n, const IndParam& slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setParam<int>("n", n);
    p->setIndParam("fast_n", fast_n);
    p->setIndParam("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(int n, int fast_n, const IndParam& slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setParam<int>("n", n);
    p->setParam<int>("fast_n", fast_n);
    p->setIndParam("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(const IndParam& n, int fast_n, int slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setIndParam("n", n);
    p->setParam<int>("fast_n", fast_n);
    p->setParam<int>("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(const IndParam& n, const IndParam& fast_n, int slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setIndParam("n", n);
    p->setIndParam("fast_n", fast_n);
    p->setParam<int>("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(const IndParam& n, int fast_n, const IndParam& slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setIndParam("n", n);
    p->setParam<int>("fast_n", fast_n);
    p->setIndParam("slow_n", slow_n);
    return Indicator(p);
}

Indicator HKU_API AMA(const IndParam& n, const IndParam& fast_n, const IndParam& slow_n) {
    IndicatorImpPtr p = make_shared<IAma>();
    p->setIndParam("n", n);
    p->setIndParam("fast_n", fast_n);
    p->setIndParam("slow_n", slow_n);
    return Indicator(p);
}

} /* namespace hku */
