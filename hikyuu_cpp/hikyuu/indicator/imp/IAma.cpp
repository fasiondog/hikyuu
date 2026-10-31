/*
 * IAma.cpp
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#include <climits>
#include <cmath>
#include "../crt/CVAL.h"
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
        HKU_ASSERT(getParam<int>("n") >= 1 && getParam<int>("n") < INT_MAX);
    } else if ("fast_n" == name) {
        HKU_ASSERT(getParam<int>("fast_n") >= 0 && getParam<int>("fast_n") < INT_MAX);
    } else if ("slow_n" == name) {
        HKU_ASSERT(getParam<int>("slow_n") >= 0 && getParam<int>("slow_n") < INT_MAX);
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
    bool seeded = std::isfinite(src[start]);
    size_t bad = seeded ? 0 : 1;
    price_t ama = seeded ? src[start] : 0.0;
    size_t first_end = start + n + 1 >= total ? total : start + n + 1;
    if (seeded) {
        _set(ama, start, 0);
        _set(er, start, 1);
    } else {
        _set(Null<price_t>(), start, 0);
        _set(Null<price_t>(), start, 1);
    }
    for (size_t i = start + 1; i < first_end; ++i) {
        if (!std::isfinite(src[i]))
            bad++;
        if (std::isfinite(src[i]) && std::isfinite(src[i - 1])) {
            vol += std::fabs(src[i] - src[i - 1]);
        }
        if (bad > 0) {
            if (seeded) {
                dst0[i] = ama;
                dst1[i] = er;
            }
            continue;
        }
        er = (vol == 0.0) ? 1.0 : (src[i] - src[start]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
        dst0[i] = ama;
        dst1[i] = er;
    }

    prevol = vol;
    for (size_t i = first_end; i < total; ++i) {
        if (!std::isfinite(src[i - n - 1]))
            bad--;
        if (!std::isfinite(src[i]))
            bad++;
        price_t added = (std::isfinite(src[i]) && std::isfinite(src[i - 1]))
                          ? std::fabs(src[i] - src[i - 1])
                          : 0.0;
        price_t removed = (std::isfinite(src[i - n]) && std::isfinite(src[i - n - 1]))
                            ? std::fabs(src[i - n] - src[i - n - 1])
                            : 0.0;
        vol = prevol + added - removed;
        prevol = vol;
        if (bad > 0) {
            if (seeded) {
                dst0[i] = ama;
                dst1[i] = er;
            }
            continue;
        }
        if (!seeded) {
            seeded = true;
            ama = src[i - n];
        }
        er = (vol == 0.0) ? 1.0 : (src[i] - src[i - n]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
        dst0[i] = ama;
        dst1[i] = er;
    }
}

size_t IAma::min_increment_start() const {
    int n = getParam<int>("n");
    // start_pos >= n + 1
    return n + 1;
}

void IAma::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    auto const* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);

    int n = getParam<int>("n");
    int fast_n = getParam<int>("fast_n");
    int slow_n = getParam<int>("slow_n");
    HKU_CHECK(start_pos >= n + 1, "start_pos: {}, n: {}", start_pos, n);

    price_t fastest = 2.0 / (fast_n + 1);
    price_t slowest = 2.0 / (slow_n + 1);
    price_t delta = fastest - slowest;

    size_t start = data.discard();
    if (start >= total) {
        return;
    }
    size_t first_end = start + n + 1 >= total ? total : start + n + 1;

    // Rebuild the (vol, ama) state at bar start_pos - 1.
    size_t begin = start_pos;
    price_t vol = 0.0;
    price_t ama = 0.0;
    bool seeded = false;
    size_t bad = 0;
    if (start_pos <= start) {
        begin = start + 1;
        seeded = std::isfinite(src[start]);
        bad = seeded ? 0 : 1;
        ama = seeded ? src[start] : 0.0;
        if (seeded) {
            dst0[start] = ama;
            dst1[start] = 1.0;
        } else {
            dst0[start] = Null<price_t>();
            dst1[start] = Null<price_t>();
        }
    } else {
        size_t win_begin = start_pos >= start + n + 1 ? start_pos - n : start + 1;
        for (size_t i = win_begin; i < start_pos; ++i) {
            if (std::isfinite(src[i]) && std::isfinite(src[i - 1])) {
                vol += std::fabs(src[i] - src[i - 1]);
            }
        }
        ama = dst0[start_pos - 1];
        seeded = std::isfinite(ama);
        size_t wstart = start_pos >= start + n + 1 ? start_pos - n - 1 : start;
        for (size_t i = wstart; i < start_pos; ++i) {
            if (!std::isfinite(src[i]))
                bad++;
        }
    }

    // resume the warm-up phase while the n-window is not full yet
    price_t er = (seeded && start_pos > start) ? dst1[start_pos - 1] : 1.0;
    for (size_t i = begin; i < first_end; ++i) {
        if (!std::isfinite(src[i]))
            bad++;
        if (std::isfinite(src[i]) && std::isfinite(src[i - 1])) {
            vol += std::fabs(src[i] - src[i - 1]);
        }
        if (bad > 0) {
            if (seeded) {
                dst0[i] = ama;
                dst1[i] = er;
            }
            continue;
        }
        er = (vol == 0.0) ? 1.0 : (src[i] - src[start]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
        dst0[i] = ama;
        dst1[i] = er;
    }

    price_t prevol = vol;
    for (size_t i = std::max(begin, first_end); i < total; ++i) {
        if (!std::isfinite(src[i - n - 1]))
            bad--;
        if (!std::isfinite(src[i]))
            bad++;
        price_t added = (std::isfinite(src[i]) && std::isfinite(src[i - 1]))
                          ? std::fabs(src[i] - src[i - 1])
                          : 0.0;
        price_t removed = (std::isfinite(src[i - n]) && std::isfinite(src[i - n - 1]))
                            ? std::fabs(src[i - n] - src[i - n - 1])
                            : 0.0;
        vol = prevol + added - removed;
        prevol = vol;
        if (bad > 0) {
            if (seeded) {
                dst0[i] = ama;
                dst1[i] = er;
            }
            continue;
        }
        if (!seeded) {
            seeded = true;
            ama = src[i - n];
        }
        er = (vol == 0.0) ? 1.0 : (src[i] - src[i - n]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
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

    // Intentional: replicate the static AMA two-phase recursion on the prefix [0, curPos] with raw
    // scalars, bitwise identical to AMA(SLICE(ind, 0, curPos + 1)) but without per-bar allocations.
    auto const* src = ind.data();
    size_t total = curPos + 1;

    price_t fastest = 2.0 / (fast_n + 1);
    price_t slowest = 2.0 / (slow_n + 1);
    price_t delta = fastest - slowest;

    price_t prevol = 0.0, vol = 0.0, er = 1.0;
    size_t start = ind.discard();
    bool seeded = std::isfinite(src[start]);
    size_t bad = seeded ? 0 : 1;
    price_t ama = seeded ? src[start] : 0.0;
    size_t first_end = start + n + 1 >= total ? total : start + n + 1;
    for (size_t i = start + 1; i < first_end; ++i) {
        if (!std::isfinite(src[i]))
            bad++;
        if (std::isfinite(src[i]) && std::isfinite(src[i - 1])) {
            vol += std::fabs(src[i] - src[i - 1]);
        }
        if (bad > 0)
            continue;
        er = (vol == 0.0) ? 1.0 : (src[i] - src[start]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
    }

    prevol = vol;
    for (size_t i = first_end; i < total; ++i) {
        if (!std::isfinite(src[i - n - 1]))
            bad--;
        if (!std::isfinite(src[i]))
            bad++;
        price_t added = (std::isfinite(src[i]) && std::isfinite(src[i - 1]))
                          ? std::fabs(src[i] - src[i - 1])
                          : 0.0;
        price_t removed = (std::isfinite(src[i - n]) && std::isfinite(src[i - n - 1]))
                            ? std::fabs(src[i - n] - src[i - n - 1])
                            : 0.0;
        vol = prevol + added - removed;
        prevol = vol;
        if (bad > 0)
            continue;
        if (!seeded) {
            seeded = true;
            ama = src[i - n];
        }
        er = (vol == 0.0) ? 1.0 : (src[i] - src[i - n]) / vol;
        if (er > 1.0)
            er = 1.0;
        if (er < -1.0)
            er = -1.0;
        price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
        ama += c * (src[i] - ama);
    }

    if (!seeded) {
        _set(Null<price_t>(), curPos, 0);
        _set(Null<price_t>(), curPos, 1);
        return;
    }
    _set(ama, curPos, 0);
    _set(er, curPos, 1);
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

    global_parallel_for_index_void(
      ind.discard(), total, [&](size_t i) { _dyn_one_circle(ind, i, n[i], fast_n[i], slow_n[i]); },
      400);

    updateDiscard();
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
