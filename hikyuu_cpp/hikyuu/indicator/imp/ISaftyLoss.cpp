/*
 * ISaftyLoss.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "../crt/SLICE.h"
#include "../crt/CVAL.h"
#include "../crt/SAFTYLOSS.h"
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

bool ISaftyLoss::check() {
    return getParam<int>("n1") >= 2 && getParam<int>("n2") >= 1;
}

void ISaftyLoss::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());
    _readyBuffer(total, 1);

    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    double p = getParam<double>("p");

    m_discard = data.discard() + n1 + n2 - 2;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t start = discard();
    for (size_t i = start; i < total; ++i) {
        price_t result = 0.0;
        for (size_t j = i + 1 - n2; j <= i; ++j) {
            price_t sum = 0.0;
            size_t num = 0;
            for (size_t k = j + 2 - n1; k <= j; ++k) {
                price_t pre = data[k - 1];
                price_t cur = data[k];
                if (pre > cur) {
                    sum += pre - cur;
                    ++num;
                }
            }

            price_t temp = data[j];
            if (num != 0) {
                temp = temp - (p * sum / num);
            }

            if (temp > result) {
                result = temp;
            }
        }

        _set(result, i);
    }
}

void ISaftyLoss::_dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2, double p) {
    HKU_IF_RETURN(n1 < 2 || n2 < 2, void());
    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator st = SAFTYLOSS(slice, n1, n2, p);
    if (st.size() > 0) {
        _set(st[st.size() - 1], curPos);
    }
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
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("p"));

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

    static const size_t minCircleLength = 400;
    size_t workerNum = ms_tg->worker_num();
    if (total < minCircleLength || workerNum == 1) {
        for (size_t i = ind.discard(); i < total; i++) {
            _dyn_one_circle(ind, i, n1[i], n2[i], p[i]);
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
        tasks.push_back(ms_tg->submit([=, &ind, &n1, &n2, &p]() {
            size_t endPos = first + circleLength;
            if (endPos > total) {
                endPos = total;
            }
            for (size_t i = circleLength * group; i < endPos; i++) {
                _dyn_one_circle(ind, i, n1[i], n2[i], p[i]);
            }
        }));
    }

    for (auto& task : tasks) {
        task.get();
    }
    _update_discard();
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
