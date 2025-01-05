/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#define EXPOERT_TA_FUNC(func) BOOST_CLASS_EXPORT(hku::Cls_##func)

#define TA_IN1_OUT1_IMP(func, func_lookback)                                         \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                             \
    Cls_##func::~Cls_##func() {}                                                     \
                                                                                     \
    void Cls_##func::_calculate(const Indicator &data) {                             \
        int lookback = func_lookback();                                              \
        size_t total = data.size();                                                  \
        if (lookback < 0) {                                                          \
            m_discard = total;                                                       \
            return;                                                                  \
        }                                                                            \
        m_discard = data.discard() + lookback;                                       \
        if (m_discard >= total) {                                                    \
            m_discard = total;                                                       \
            return;                                                                  \
        }                                                                            \
                                                                                     \
        auto const *src = data.data();                                               \
        auto *dst = this->data();                                                    \
        int outBegIdx;                                                               \
        int outNbElement;                                                            \
        func(m_discard, total - 1, src, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total); \
    }                                                                                \
                                                                                     \
    Indicator HKU_API func() {                                                       \
        return Indicator(make_shared<Cls_##func>());                                 \
    }

#define TA_IN1_OUT1_INT_IMP(func, func_lookback)                                     \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                             \
    Cls_##func::~Cls_##func() {}                                                     \
                                                                                     \
    void Cls_##func::_calculate(const Indicator &data) {                             \
        int lookback = func_lookback();                                              \
        size_t total = data.size();                                                  \
        if (lookback < 0) {                                                          \
            m_discard = total;                                                       \
            return;                                                                  \
        }                                                                            \
                                                                                     \
        m_discard = data.discard() + lookback;                                       \
        if (m_discard >= total) {                                                    \
            m_discard = total;                                                       \
            return;                                                                  \
        }                                                                            \
                                                                                     \
        auto const *src = data.data();                                               \
        std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);                 \
        int outBegIdx;                                                               \
        int outNbElement;                                                            \
        func(m_discard, total - 1, src, &outBegIdx, &outNbElement, buf.get());       \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total); \
        m_discard = outBegIdx;                                                       \
        auto *dst = this->data();                                                    \
        dst = dst + outBegIdx;                                                       \
        for (int i = 0; i < outNbElement; ++i) {                                     \
            dst[i] = buf[i];                                                         \
        }                                                                            \
    }                                                                                \
                                                                                     \
    Indicator HKU_API func() {                                                       \
        return Indicator(make_shared<Cls_##func>());                                 \
    }

#define TA_IN1_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)             \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                    \
        setParam<int>("n", period);                                                        \
    }                                                                                      \
    Cls_##func::~Cls_##func() {}                                                           \
                                                                                           \
    void Cls_##func::_checkParam(const string &name) const {                               \
        if (name == "n") {                                                                 \
            int n = getParam<int>("n");                                                    \
            HKU_ASSERT(n >= period_min && n <= period_max);                                \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        int n = getParam<int>("n");                                                        \
        int lookback = func_lookback(n);                                                   \
        size_t total = data.size();                                                        \
        if (lookback < 0) {                                                                \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        m_discard = data.discard() + lookback;                                             \
        if (m_discard >= total) {                                                          \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        auto const *src = data.data();                                                     \
        auto *dst = this->data();                                                          \
                                                                                           \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement, dst + m_discard);    \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);       \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) { \
        int back = func_lookback(step);                                                    \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                     \
                                                                                           \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(curPos);                \
        auto const *src = ind.data();                                                      \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, buf.get());      \
        if (outNbElement >= 1) {                                                           \
            _set(buf.get()[outNbElement - 1], curPos);                                     \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(int n) {                                                        \
        auto p = make_shared<Cls_##func>();                                                \
        p->setParam<int>("n", n);                                                          \
        return Indicator(p);                                                               \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(const IndParam &n) {                                            \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                     \
        p->setIndParam("n", n);                                                            \
        return Indicator(p);                                                               \
    }

#define TA_IN1_OUT1_INT_N_IMP(func, func_lookback, period, period_min, period_max)         \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                    \
        setParam<int>("n", period);                                                        \
    }                                                                                      \
    Cls_##func::~Cls_##func() {}                                                           \
                                                                                           \
    void Cls_##func::_checkParam(const string &name) const {                               \
        if (name == "n") {                                                                 \
            int n = getParam<int>("n");                                                    \
            HKU_ASSERT(n >= period_min && n <= period_max);                                \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        int n = getParam<int>("n");                                                        \
        int lookback = func_lookback(n);                                                   \
        size_t total = data.size();                                                        \
        if (lookback < 0) {                                                                \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        m_discard = data.discard() + lookback;                                             \
        if (m_discard >= total) {                                                          \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        auto const *src = data.data();                                                     \
        std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);                       \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement, buf.get());          \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);       \
        m_discard = outBegIdx;                                                             \
        auto *dst = this->data();                                                          \
        dst = dst + outBegIdx;                                                             \
        for (int i = 0; i < outNbElement; ++i) {                                           \
            dst[i] = buf[i];                                                               \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) { \
        int back = func_lookback(step);                                                    \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                     \
                                                                                           \
        std::unique_ptr<int[]> buf = std::make_unique<int[]>(curPos);                      \
        auto const *src = ind.data();                                                      \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, buf.get());      \
        if (outNbElement >= 1) {                                                           \
            _set(buf.get()[outNbElement - 1], curPos);                                     \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(int n) {                                                        \
        auto p = make_shared<Cls_##func>();                                                \
        p->setParam<int>("n", n);                                                          \
        return Indicator(p);                                                               \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(const IndParam &n) {                                            \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                     \
        p->setIndParam("n", n);                                                            \
        return Indicator(p);                                                               \
    }

#define TA_IN1_OUT2_IMP(func, func_lookback)                                         \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {}                             \
    Cls_##func::~Cls_##func() {}                                                     \
    void Cls_##func::_calculate(const Indicator &data) {                             \
        size_t total = data.size();                                                  \
        int lookback = func_lookback();                                              \
        if (lookback < 0) {                                                          \
            m_discard = total;                                                       \
            return;                                                                  \
        }                                                                            \
        m_discard = data.discard() + lookback;                                       \
        if (m_discard >= total) {                                                    \
            m_discard = total;                                                       \
            return;                                                                  \
        }                                                                            \
                                                                                     \
        auto const *src = data.data();                                               \
        auto *dst0 = this->data(0);                                                  \
        auto *dst1 = this->data(1);                                                  \
                                                                                     \
        int outBegIdx;                                                               \
        int outNbElement;                                                            \
        func(m_discard, total - 1, src, &outBegIdx, &outNbElement, dst0 + m_discard, \
             dst1 + m_discard);                                                      \
        HKU_ASSERT(outBegIdx == m_discard && (outBegIdx + outNbElement) <= total);   \
    }                                                                                \
                                                                                     \
    Indicator HKU_API func() {                                                       \
        return Indicator(make_shared<Cls_##func>());                                 \
    }

#define TA_IN1_OUT2_INT_N_IMP(func, func_lookback, period, period_min, period_max)         \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                                    \
        setParam<int>("n", period);                                                        \
    }                                                                                      \
    Cls_##func::~Cls_##func() {}                                                           \
                                                                                           \
    void Cls_##func::_checkParam(const string &name) const {                               \
        if (name == "n") {                                                                 \
            int n = getParam<int>("n");                                                    \
            HKU_ASSERT(n >= period_min && n <= period_max);                                \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        int n = getParam<int>("n");                                                        \
        int lookback = func_lookback(n);                                                   \
        size_t total = data.size();                                                        \
        if (lookback < 0) {                                                                \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        m_discard = data.discard() + lookback;                                             \
        if (m_discard >= total) {                                                          \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        auto const *src = data.data();                                                     \
        std::unique_ptr<int[]> buf = std::make_unique<int[]>(2 * (total));                 \
        int *buf0 = buf.get();                                                             \
        int *buf1 = buf0 + total;                                                          \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement, buf0, buf1);         \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);       \
        m_discard = outBegIdx;                                                             \
        auto *dst0 = this->data(0) + outBegIdx;                                            \
        auto *dst1 = this->data(1) + outBegIdx;                                            \
        for (int i = 0; i < outNbElement; ++i) {                                           \
            dst0[i] = buf0[i];                                                             \
            dst1[i] = buf1[i];                                                             \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) { \
        int back = func_lookback(step);                                                    \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                     \
                                                                                           \
        std::unique_ptr<int[]> buf = std::make_unique<int[]>(2 * curPos);                  \
        int *buf0 = buf.get();                                                             \
        int *buf1 = buf0 + curPos;                                                         \
        auto const *src = ind.data();                                                      \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, buf0, buf1);     \
        if (outNbElement >= 1) {                                                           \
            _set(buf0[outNbElement - 1], curPos, 0);                                       \
            _set(buf1[outNbElement - 1], curPos, 1);                                       \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(int n) {                                                        \
        auto p = make_shared<Cls_##func>();                                                \
        p->setParam<int>("n", n);                                                          \
        return Indicator(p);                                                               \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(const IndParam &n) {                                            \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                     \
        p->setIndParam("n", n);                                                            \
        return Indicator(p);                                                               \
    }

#define TA_IN1_OUT2_N_IMP(func, func_lookback, period, period_min, period_max)             \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                                    \
        setParam<int>("n", period);                                                        \
    }                                                                                      \
    Cls_##func::~Cls_##func() {}                                                           \
                                                                                           \
    void Cls_##func::_checkParam(const string &name) const {                               \
        if (name == "n") {                                                                 \
            int n = getParam<int>("n");                                                    \
            HKU_ASSERT(n >= period_min && n <= period_max);                                \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        int n = getParam<int>("n");                                                        \
        size_t total = data.size();                                                        \
        int lookback = func_lookback(n);                                                   \
        if (lookback < 0) {                                                                \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
        m_discard = data.discard() + lookback;                                             \
        if (m_discard >= total) {                                                          \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        auto const *src = data.data();                                                     \
        auto *dst0 = this->data(0);                                                        \
        auto *dst1 = this->data(1);                                                        \
                                                                                           \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement, dst0 + m_discard,    \
             dst1 + m_discard);                                                            \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);       \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) { \
        int back = func_lookback(step);                                                    \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                     \
                                                                                           \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * curPos);            \
        double *dst0 = buf.get();                                                          \
        double *ds1 = dst0 + curPos;                                                       \
        auto const *src = ind.data();                                                      \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, dst0, ds1);      \
        if (outNbElement >= 1) {                                                           \
            _set(dst0[outNbElement - 1], curPos, 0);                                       \
            _set(ds1[outNbElement - 1], curPos, 1);                                        \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(int n) {                                                        \
        auto p = make_shared<Cls_##func>();                                                \
        p->setParam<int>("n", n);                                                          \
        return Indicator(p);                                                               \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(const IndParam &n) {                                            \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                     \
        p->setIndParam("n", n);                                                            \
        return Indicator(p);                                                               \
    }

#define TA_IN1_OUT3_N_IMP(func, func_lookback, period, period_min, period_max)             \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 3) {                                    \
        setParam<int>("n", period);                                                        \
    }                                                                                      \
    Cls_##func::~Cls_##func() {}                                                           \
                                                                                           \
    void Cls_##func::_checkParam(const string &name) const {                               \
        if (name == "n") {                                                                 \
            int n = getParam<int>("n");                                                    \
            HKU_ASSERT(n >= period_min && n <= period_max);                                \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        int n = getParam<int>("n");                                                        \
        size_t total = data.size();                                                        \
        int lookback = func_lookback(n);                                                   \
        if (lookback < 0) {                                                                \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
        m_discard = data.discard() + lookback;                                             \
        if (m_discard >= total) {                                                          \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        auto const *src = data.data();                                                     \
        auto *dst0 = this->data(0);                                                        \
        auto *dst1 = this->data(1);                                                        \
        auto *dst2 = this->data(2);                                                        \
                                                                                           \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement, dst0 + m_discard,    \
             dst1 + m_discard, dst2 + m_discard);                                          \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);       \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) { \
        int back = func_lookback(step);                                                    \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                     \
                                                                                           \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * curPos);            \
        double *dst0 = buf.get();                                                          \
        double *ds1 = dst0 + curPos;                                                       \
        double *ds2 = ds1 + curPos;                                                        \
        auto const *src = ind.data();                                                      \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, dst0, ds1, ds2); \
        if (outNbElement >= 1) {                                                           \
            _set(dst0[outNbElement - 1], curPos, 0);                                       \
            _set(ds1[outNbElement - 1], curPos, 1);                                        \
            _set(ds2[outNbElement - 1], curPos, 2);                                        \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(int n) {                                                        \
        auto p = make_shared<Cls_##func>();                                                \
        p->setParam<int>("n", n);                                                          \
        return Indicator(p);                                                               \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(const IndParam &n) {                                            \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                     \
        p->setIndParam("n", n);                                                            \
        return Indicator(p);                                                               \
    }

#define TA_IN2_OUT1_IMP(func, func_lookback)                                                      \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                           \
        setParam<bool>("fill_null", true);                                                        \
    }                                                                                             \
    Cls_##func::Cls_##func(const Indicator &ref_ind, bool fill_null)                              \
    : IndicatorImp(#func, 1), m_ref_ind(ref_ind) {                                                \
        setParam<bool>("fill_null", fill_null);                                                   \
    }                                                                                             \
    Cls_##func::~Cls_##func() {}                                                                  \
    IndicatorImpPtr Cls_##func::_clone() {                                                        \
        auto p = make_shared<Cls_##func>();                                                       \
        p->m_ref_ind = m_ref_ind.clone();                                                         \
        return p;                                                                                 \
    }                                                                                             \
                                                                                                  \
    void Cls_##func::_calculate(const Indicator &ind) {                                           \
        size_t total = ind.size();                                                                \
        HKU_IF_RETURN(total == 0, void());                                                        \
                                                                                                  \
        _readyBuffer(total, 1);                                                                   \
                                                                                                  \
        auto k = getContext();                                                                    \
        m_ref_ind.setContext(k);                                                                  \
        Indicator ref = m_ref_ind;                                                                \
        auto dates = ref.getDatetimeList();                                                       \
        if (dates.empty()) {                                                                      \
            if (ref.size() > ind.size()) {                                                        \
                ref = SLICE(ref, ref.size() - ind.size(), ref.size());                            \
            } else if (ref.size() < ind.size()) {                                                 \
                ref = CVAL(ind, 0.) + ref;                                                        \
            }                                                                                     \
        } else if (m_ref_ind.size() != ind.size()) {                                              \
            ref = ALIGN(m_ref_ind, ind, getParam<bool>("fill_null"));                             \
        }                                                                                         \
                                                                                                  \
        int lookback = func_lookback();                                                           \
        if (lookback < 0) {                                                                       \
            m_discard = total;                                                                    \
            return;                                                                               \
        }                                                                                         \
                                                                                                  \
        size_t in_discard = std::max(ind.discard(), ref.discard());                               \
        m_discard = lookback + in_discard;                                                        \
        if (m_discard >= total) {                                                                 \
            m_discard = total;                                                                    \
            return;                                                                               \
        }                                                                                         \
                                                                                                  \
        const auto *src0 = ind.data();                                                            \
        const auto *src1 = ref.data();                                                            \
        auto *dst = this->data();                                                                 \
        int outBegIdx;                                                                            \
        int outNbElement;                                                                         \
        func(m_discard, total - 1, src0, src1, &outBegIdx, &outNbElement, dst + m_discard);       \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);              \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(bool fill_null = true) {                                               \
        auto p = make_shared<Cls_##func>();                                                       \
        p->setParam<bool>("fill_null", fill_null);                                                \
        return Indicator(p);                                                                      \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(const Indicator &ind1, const Indicator &ind2, bool fill_null = true) { \
        auto p = make_shared<Cls_##func>(ind2, fill_null);                                        \
        Indicator result(p);                                                                      \
        return result(ind1);                                                                      \
    }

#define TA_IN2_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)                 \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                        \
        setParam<int>("n", period);                                                            \
        setParam<bool>("fill_null", true);                                                     \
    }                                                                                          \
                                                                                               \
    Cls_##func::Cls_##func(int n, bool fill_null) : IndicatorImp(#func, 1) {                   \
        setParam<int>("n", n);                                                                 \
        setParam<bool>("fill_null", fill_null);                                                \
    }                                                                                          \
                                                                                               \
    Cls_##func::Cls_##func(const Indicator &ref_ind, int n, bool fill_null)                    \
    : IndicatorImp(#func, 1), m_ref_ind(ref_ind) {                                             \
        setParam<int>("n", n);                                                                 \
        setParam<bool>("fill_null", fill_null);                                                \
    }                                                                                          \
                                                                                               \
    Cls_##func::~Cls_##func() {}                                                               \
                                                                                               \
    void Cls_##func::_checkParam(const string &name) const {                                   \
        if ("n" == name) {                                                                     \
            int n = getParam<int>("n");                                                        \
            HKU_ASSERT(n >= period_min && n <= period_max);                                    \
        }                                                                                      \
    }                                                                                          \
                                                                                               \
    IndicatorImpPtr Cls_##func::_clone() {                                                     \
        auto p = make_shared<Cls_##func>();                                                    \
        p->m_ref_ind = m_ref_ind.clone();                                                      \
        return p;                                                                              \
    }                                                                                          \
                                                                                               \
    void Cls_##func::_calculate(const Indicator &ind) {                                        \
        size_t total = ind.size();                                                             \
        HKU_IF_RETURN(total == 0, void());                                                     \
                                                                                               \
        _readyBuffer(total, 1);                                                                \
                                                                                               \
        auto k = getContext();                                                                 \
        m_ref_ind.setContext(k);                                                               \
        Indicator ref = m_ref_ind;                                                             \
        auto dates = ref.getDatetimeList();                                                    \
        if (dates.empty()) {                                                                   \
            if (ref.size() > ind.size()) {                                                     \
                ref = SLICE(ref, ref.size() - ind.size(), ref.size());                         \
            } else if (ref.size() < ind.size()) {                                              \
                ref = CVAL(ind, 0.) + ref;                                                     \
            }                                                                                  \
        } else if (m_ref_ind.size() != ind.size()) {                                           \
            ref = ALIGN(m_ref_ind, ind, getParam<bool>("fill_null"));                          \
        }                                                                                      \
                                                                                               \
        int n = getParam<int>("n");                                                            \
        int lookback = func_lookback(n);                                                       \
        if (lookback < 0) {                                                                    \
            m_discard = total;                                                                 \
            return;                                                                            \
        }                                                                                      \
                                                                                               \
        size_t in_discard = std::max(ind.discard(), ref.discard());                            \
        m_discard = lookback + in_discard;                                                     \
        if (m_discard >= total) {                                                              \
            m_discard = total;                                                                 \
            return;                                                                            \
        }                                                                                      \
                                                                                               \
        const auto *src0 = ind.data();                                                         \
        const auto *src1 = ref.data();                                                         \
        auto *dst = this->data();                                                              \
        int outBegIdx;                                                                         \
        int outNbElement;                                                                      \
        func(m_discard, total - 1, src0, src1, n, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);           \
    }                                                                                          \
                                                                                               \
    Indicator HKU_API func(int n, bool fill_null = true) {                                     \
        return Indicator(make_shared<Cls_##func>(n, fill_null));                               \
    }                                                                                          \
                                                                                               \
    Indicator HKU_API func(const Indicator &ind1, const Indicator &ind2, int n,                \
                           bool fill_null = true) {                                            \
        auto p = make_shared<Cls_##func>(ind2, n, fill_null);                                  \
        HKU_WARN_IF(ind2.size() == 0, "The lenght of ind2 is zero!");                          \
        Indicator result(p);                                                                   \
        return result(ind1);                                                                   \
    }

#define TA_OHLC_OUT1_IMP(func, func_lookback)                                           \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k) : IndicatorImp(#func, 1) {                   \
        setParam<KData>("kdata", k);                                                    \
        Cls_##func::_calculate(Indicator());                                            \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_calculate(const Indicator &data) {                                \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                         \
                    "The input is ignored because {} depends on the context!", m_name); \
                                                                                        \
        KData k = getContext();                                                         \
        size_t total = k.size();                                                        \
        HKU_IF_RETURN(total == 0, void());                                              \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
        int lookback = func_lookback();                                                 \
        if (lookback < 0 || lookback >= total) {                                        \
            m_discard = total;                                                          \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);          \
        double *open = buf.get();                                                       \
        double *high = open + total;                                                    \
        double *low = high + total;                                                     \
        double *close = low + total;                                                    \
        for (size_t i = 0; i < total; ++i) {                                            \
            open[i] = kptr[i].openPrice;                                                \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
            close[i] = kptr[i].closePrice;                                              \
        }                                                                               \
                                                                                        \
        m_discard = lookback;                                                           \
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(m_discard, total - 1, open, high, low, close, &outBegIdx, &outNbElement,   \
             dst + m_discard);                                                          \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);    \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func() {                                                          \
        return make_shared<Cls_##func>()->calculate();                                  \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k) {                                            \
        return Indicator(make_shared<Cls_##func>(k));                                   \
    }

#define TA_OHLC_OUT1_INT_IMP(func, func_lookback)                                       \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k) : IndicatorImp(#func, 1) {                   \
        setParam<KData>("kdata", k);                                                    \
        Cls_##func::_calculate(Indicator());                                            \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_calculate(const Indicator &data) {                                \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                         \
                    "The input is ignored because {} depends on the context!", m_name); \
                                                                                        \
        KData k = getContext();                                                         \
        size_t total = k.size();                                                        \
        HKU_IF_RETURN(total == 0, void());                                              \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
                                                                                        \
        int lookback = func_lookback();                                                 \
        if (lookback < 0 || lookback >= total) {                                        \
            m_discard = total;                                                          \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);          \
        double *open = buf.get();                                                       \
        double *high = open + total;                                                    \
        double *low = high + total;                                                     \
        double *close = low + total;                                                    \
        for (size_t i = 0; i < total; ++i) {                                            \
            open[i] = kptr[i].openPrice;                                                \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
            close[i] = kptr[i].closePrice;                                              \
        }                                                                               \
                                                                                        \
        std::unique_ptr<int[]> outbuf = std::make_unique<int[]>(total);                 \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        m_discard = lookback;                                                           \
        func(m_discard, total - 1, open, high, low, close, &outBegIdx, &outNbElement,   \
             outbuf.get());                                                             \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);    \
        auto *dst = this->data() + outBegIdx;                                           \
        for (size_t i = 0; i < outNbElement; ++i) {                                     \
            dst[i] = outbuf[i];                                                         \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func() {                                                          \
        return make_shared<Cls_##func>()->calculate();                                  \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k) {                                            \
        return Indicator(make_shared<Cls_##func>(k));                                   \
    }

#define TA_OHLC_OUT1_INT_P1_D_IMP(func, func_lookback, param1, param1_value, param1_min,          \
                                  param1_max)                                                     \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                           \
        setParam<double>(#param1, param1_value);                                                  \
    }                                                                                             \
                                                                                                  \
    Cls_##func::Cls_##func(const KData &k, double p) : IndicatorImp(#func, 1) {                   \
        setParam<KData>("kdata", k);                                                              \
        setParam<double>(#param1, p);                                                             \
        Cls_##func::_calculate(Indicator());                                                      \
    }                                                                                             \
                                                                                                  \
    void Cls_##func::_checkParam(const string &name) const {                                      \
        if (name == #param1) {                                                                    \
            double p = getParam<double>(#param1);                                                 \
            HKU_ASSERT(p >= param1_min && p <= param1_max);                                       \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    void Cls_##func::_calculate(const Indicator &data) {                                          \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                                   \
                    "The input is ignored because {} depends on the context!", m_name);           \
                                                                                                  \
        KData k = getContext();                                                                   \
        size_t total = k.size();                                                                  \
        HKU_IF_RETURN(total == 0, void());                                                        \
                                                                                                  \
        _readyBuffer(total, 1);                                                                   \
                                                                                                  \
        int lookback = func_lookback(param1_value);                                               \
        if (lookback < 0 || lookback >= total) {                                                  \
            m_discard = total;                                                                    \
            return;                                                                               \
        }                                                                                         \
                                                                                                  \
        const KRecord *kptr = k.data();                                                           \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);                    \
        double *open = buf.get();                                                                 \
        double *high = open + total;                                                              \
        double *low = high + total;                                                               \
        double *close = low + total;                                                              \
        for (size_t i = 0; i < total; ++i) {                                                      \
            open[i] = kptr[i].openPrice;                                                          \
            high[i] = kptr[i].highPrice;                                                          \
            low[i] = kptr[i].lowPrice;                                                            \
            close[i] = kptr[i].closePrice;                                                        \
        }                                                                                         \
                                                                                                  \
        std::unique_ptr<int[]> outbuf = std::make_unique<int[]>(total);                           \
        int outBegIdx;                                                                            \
        int outNbElement;                                                                         \
        m_discard = lookback;                                                                     \
        func(m_discard, total - 1, open, high, low, close, getParam<double>(#param1), &outBegIdx, \
             &outNbElement, outbuf.get());                                                        \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);              \
        auto *dst = this->data() + outBegIdx;                                                     \
        for (size_t i = 0; i < outNbElement; ++i) {                                               \
            dst[i] = outbuf[i];                                                                   \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(double p) {                                                            \
        auto ptr = make_shared<Cls_##func>();                                                     \
        ptr->setParam<double>(#param1, p);                                                        \
        ptr->calculate();                                                                         \
        return Indicator(ptr);                                                                    \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(const KData &k, double p) {                                            \
        return Indicator(make_shared<Cls_##func>(k, p));                                          \
    }

#define TA_HLCV_OUT1_IMP(func, func_lookback)                                           \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k) : IndicatorImp(#func, 1) {                   \
        setParam<KData>("kdata", k);                                                    \
        Cls_##func::_calculate(Indicator());                                            \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_calculate(const Indicator &data) {                                \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                         \
                    "The input is ignored because {} depends on the context!", m_name); \
                                                                                        \
        KData k = getContext();                                                         \
        size_t total = k.size();                                                        \
        HKU_IF_RETURN(total == 0, void());                                              \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
        int lookback = func_lookback();                                                 \
        if (lookback < 0 || lookback >= total) {                                        \
            m_discard = total;                                                          \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        double *close = low + total;                                                    \
        double *vol = close + total;                                                    \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
            close[i] = kptr[i].closePrice;                                              \
            vol[i] = kptr[i].transCount;                                                \
        }                                                                               \
                                                                                        \
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        m_discard = lookback;                                                           \
        func(m_discard, total - 1, high, low, close, vol, &outBegIdx, &outNbElement,    \
             dst + m_discard);                                                          \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);    \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func() {                                                          \
        return make_shared<Cls_##func>()->calculate();                                  \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k) {                                            \
        return Indicator(make_shared<Cls_##func>(k));                                   \
    }

#define TA_HL_OUT1_IMP(func, func_lookback)                                                \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                   \
                                                                                           \
    Cls_##func::Cls_##func(const KData &k) : IndicatorImp(#func, 1) {                      \
        setParam<KData>("kdata", k);                                                       \
        Cls_##func::_calculate(Indicator());                                               \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                            \
                    "The input is ignored because {} depends on the context!", m_name);    \
                                                                                           \
        KData k = getContext();                                                            \
        size_t total = k.size();                                                           \
        HKU_IF_RETURN(total == 0, void());                                                 \
                                                                                           \
        _readyBuffer(total, 1);                                                            \
        int lookback = func_lookback();                                                    \
        if (lookback < 0 || lookback >= total) {                                           \
            m_discard = total;                                                             \
            return;                                                                        \
        }                                                                                  \
                                                                                           \
        const KRecord *kptr = k.data();                                                    \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);             \
        double *high = buf.get();                                                          \
        double *low = high + total;                                                        \
        for (size_t i = 0; i < total; ++i) {                                               \
            high[i] = kptr[i].highPrice;                                                   \
            low[i] = kptr[i].lowPrice;                                                     \
        }                                                                                  \
                                                                                           \
        auto *dst = this->data();                                                          \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        m_discard = lookback;                                                              \
        func(m_discard, total - 1, high, low, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);       \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func() {                                                             \
        return make_shared<Cls_##func>()->calculate();                                     \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func(const KData &k) {                                               \
        return Indicator(make_shared<Cls_##func>(k));                                      \
    }

#define TA_CV_OUT1_IMP(func, func_lookback)                                                 \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                    \
                                                                                            \
    Cls_##func::Cls_##func(const KData &k) : IndicatorImp(#func, 1) {                       \
        setParam<KData>("kdata", k);                                                        \
        Cls_##func::_calculate(Indicator());                                                \
    }                                                                                       \
                                                                                            \
    void Cls_##func::_calculate(const Indicator &data) {                                    \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                             \
                    "The input is ignored because {} depends on the context!", m_name);     \
                                                                                            \
        KData k = getContext();                                                             \
        size_t total = k.size();                                                            \
        HKU_IF_RETURN(total == 0, void());                                                  \
                                                                                            \
        _readyBuffer(total, 1);                                                             \
        int lookback = func_lookback();                                                     \
        if (lookback < 0 || lookback >= total) {                                            \
            m_discard = 0;                                                                  \
            return;                                                                         \
        }                                                                                   \
                                                                                            \
        const KRecord *kptr = k.data();                                                     \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);              \
        double *close = buf.get();                                                          \
        double *vol = close + total;                                                        \
        for (size_t i = 0; i < total; ++i) {                                                \
            close[i] = kptr[i].closePrice;                                                  \
            vol[i] = kptr[i].transCount;                                                    \
        }                                                                                   \
                                                                                            \
        auto *dst = this->data();                                                           \
        int outBegIdx;                                                                      \
        int outNbElement;                                                                   \
        m_discard = lookback;                                                               \
        func(m_discard, total - 1, close, vol, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);        \
    }                                                                                       \
                                                                                            \
    Indicator HKU_API func() {                                                              \
        return make_shared<Cls_##func>()->calculate();                                      \
    }                                                                                       \
                                                                                            \
    Indicator HKU_API func(const KData &k) {                                                \
        return Indicator(make_shared<Cls_##func>(k));                                       \
    }

#define TA_HLC_OUT1_IMP(func, func_lookback)                                                      \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                          \
                                                                                                  \
    Cls_##func::Cls_##func(const KData &k) : IndicatorImp(#func, 1) {                             \
        setParam<KData>("kdata", k);                                                              \
        Cls_##func::_calculate(Indicator());                                                      \
    }                                                                                             \
                                                                                                  \
    void Cls_##func::_calculate(const Indicator &data) {                                          \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                                   \
                    "The input is ignored because {} depends on the context!", m_name);           \
                                                                                                  \
        KData k = getContext();                                                                   \
        size_t total = k.size();                                                                  \
        HKU_IF_RETURN(total == 0, void());                                                        \
                                                                                                  \
        _readyBuffer(total, 1);                                                                   \
        int lookback = func_lookback();                                                           \
        if (lookback < 0 || lookback >= total) {                                                  \
            m_discard = total;                                                                    \
            return;                                                                               \
        }                                                                                         \
                                                                                                  \
        const KRecord *kptr = k.data();                                                           \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);                    \
        double *high = buf.get();                                                                 \
        double *low = high + total;                                                               \
        double *close = low + total;                                                              \
        for (size_t i = 0; i < total; ++i) {                                                      \
            high[i] = kptr[i].highPrice;                                                          \
            low[i] = kptr[i].lowPrice;                                                            \
            close[i] = kptr[i].closePrice;                                                        \
        }                                                                                         \
                                                                                                  \
        m_discard = lookback;                                                                     \
        auto *dst = this->data();                                                                 \
        int outBegIdx;                                                                            \
        int outNbElement;                                                                         \
        func(m_discard, total - 1, high, low, close, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);              \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func() {                                                                    \
        return make_shared<Cls_##func>()->calculate();                                            \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(const KData &k) {                                                      \
        return Indicator(make_shared<Cls_##func>(k));                                             \
    }

#define TA_HLC_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)          \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                 \
        setParam<int>("n", period);                                                     \
    }                                                                                   \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 1) {            \
        setParam<KData>("kdata", k);                                                    \
        setParam<int>("n", n);                                                          \
        Cls_##func::_calculate(Indicator());                                            \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_checkParam(const string &name) const {                            \
        if (name == "n") {                                                              \
            int n = getParam<int>("n");                                                 \
            HKU_ASSERT(n >= period_min && n <= period_max);                             \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_calculate(const Indicator &data) {                                \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                         \
                    "The input is ignored because {} depends on the context!", m_name); \
                                                                                        \
        KData k = getContext();                                                         \
        size_t total = k.size();                                                        \
        HKU_IF_RETURN(total == 0, void());                                              \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
        int n = getParam<int>("n");                                                     \
        int back = func_lookback(n);                                                    \
        if (back < 0 || back >= total) {                                                \
            m_discard = total;                                                          \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        double *close = low + total;                                                    \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
            close[i] = kptr[i].closePrice;                                              \
        }                                                                               \
                                                                                        \
        m_discard = back;                                                               \
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(m_discard, total - 1, high, low, close, n, &outBegIdx, &outNbElement,      \
             dst + m_discard);                                                          \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);    \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(int n) {                                                     \
        auto p = make_shared<Cls_##func>();                                             \
        p->setParam<int>("n", n);                                                       \
        p->calculate();                                                                 \
        return Indicator(p);                                                            \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k, int n) {                                     \
        return Indicator(make_shared<Cls_##func>(k, n));                                \
    }

#define TA_HLCV_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)         \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                 \
        setParam<int>("n", period);                                                     \
    }                                                                                   \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 1) {            \
        setParam<KData>("kdata", k);                                                    \
        setParam<int>("n", n);                                                          \
        Cls_##func::_calculate(Indicator());                                            \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_checkParam(const string &name) const {                            \
        if (name == "n") {                                                              \
            int n = getParam<int>("n");                                                 \
            HKU_ASSERT(n >= period_min && n <= period_max);                             \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_calculate(const Indicator &data) {                                \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                         \
                    "The input is ignored because {} depends on the context!", m_name); \
                                                                                        \
        KData k = getContext();                                                         \
        size_t total = k.size();                                                        \
        HKU_IF_RETURN(total == 0, void());                                              \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
                                                                                        \
        int n = getParam<int>("n");                                                     \
        int back = func_lookback(n);                                                    \
        if (back < 0 || back >= total) {                                                \
            m_discard = total;                                                          \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        double *close = low + total;                                                    \
        double *vol = close + total;                                                    \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
            close[i] = kptr[i].closePrice;                                              \
            vol[i] = kptr[i].transCount;                                                \
        }                                                                               \
                                                                                        \
        m_discard = back;                                                               \
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(m_discard, total - 1, high, low, close, vol, n, &outBegIdx, &outNbElement, \
             dst + m_discard);                                                          \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);    \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(int n) {                                                     \
        auto p = make_shared<Cls_##func>();                                             \
        p->setParam<int>("n", n);                                                       \
        p->calculate();                                                                 \
        return Indicator(p);                                                            \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k, int n) {                                     \
        return Indicator(make_shared<Cls_##func>(k, n));                                \
    }

#define TA_HL_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)                 \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                       \
        setParam<int>("n", period);                                                           \
    }                                                                                         \
                                                                                              \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 1) {                  \
        setParam<KData>("kdata", k);                                                          \
        setParam<int>("n", n);                                                                \
        Cls_##func::_calculate(Indicator());                                                  \
    }                                                                                         \
                                                                                              \
    void Cls_##func::_checkParam(const string &name) const {                                  \
        if (name == "n") {                                                                    \
            int n = getParam<int>("n");                                                       \
            HKU_ASSERT(n >= period_min && n <= period_max);                                   \
        }                                                                                     \
    }                                                                                         \
                                                                                              \
    void Cls_##func::_calculate(const Indicator &data) {                                      \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                               \
                    "The input is ignored because {} depends on the context!", m_name);       \
                                                                                              \
        KData k = getContext();                                                               \
        size_t total = k.size();                                                              \
        HKU_IF_RETURN(total == 0, void());                                                    \
                                                                                              \
        _readyBuffer(total, 1);                                                               \
        int n = getParam<int>("n");                                                           \
        int back = func_lookback(n);                                                          \
        if (back < 0 || back >= total) {                                                      \
            m_discard = total;                                                                \
            return;                                                                           \
        }                                                                                     \
                                                                                              \
        const KRecord *kptr = k.data();                                                       \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);                \
        double *high = buf.get();                                                             \
        double *low = high + total;                                                           \
        for (size_t i = 0; i < total; ++i) {                                                  \
            high[i] = kptr[i].highPrice;                                                      \
            low[i] = kptr[i].lowPrice;                                                        \
        }                                                                                     \
                                                                                              \
        m_discard = back;                                                                     \
        auto *dst = this->data();                                                             \
        int outBegIdx;                                                                        \
        int outNbElement;                                                                     \
        func(m_discard, total - 1, high, low, n, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);          \
    }                                                                                         \
                                                                                              \
    Indicator HKU_API func(int n) {                                                           \
        auto p = make_shared<Cls_##func>();                                                   \
        p->setParam<int>("n", n);                                                             \
        p->calculate();                                                                       \
        return Indicator(p);                                                                  \
    }                                                                                         \
                                                                                              \
    Indicator HKU_API func(const KData &k, int n) {                                           \
        return Indicator(make_shared<Cls_##func>(k, n));                                      \
    }

#define TA_HL_OUT2_N_IMP(func, func_lookback, period, period_min, period_max)                 \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                                       \
        setParam<int>("n", period);                                                           \
    }                                                                                         \
                                                                                              \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 2) {                  \
        setParam<KData>("kdata", k);                                                          \
        setParam<int>("n", n);                                                                \
        Cls_##func::_calculate(Indicator());                                                  \
    }                                                                                         \
                                                                                              \
    void Cls_##func::_checkParam(const string &name) const {                                  \
        if (name == "n") {                                                                    \
            int n = getParam<int>("n");                                                       \
            HKU_ASSERT(n >= period_min && n <= period_max);                                   \
        }                                                                                     \
    }                                                                                         \
                                                                                              \
    void Cls_##func::_calculate(const Indicator &data) {                                      \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                               \
                    "The input is ignored because {} depends on the context!", m_name);       \
                                                                                              \
        KData k = getContext();                                                               \
        size_t total = k.size();                                                              \
        HKU_IF_RETURN(total == 0, void());                                                    \
                                                                                              \
        _readyBuffer(total, 2);                                                               \
        int n = getParam<int>("n");                                                           \
        int back = func_lookback(n);                                                          \
        if (back < 0 || back >= total) {                                                      \
            m_discard = total;                                                                \
            return;                                                                           \
        }                                                                                     \
                                                                                              \
        const KRecord *kptr = k.data();                                                       \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);                \
        double *high = buf.get();                                                             \
        double *low = high + total;                                                           \
        for (size_t i = 0; i < total; ++i) {                                                  \
            high[i] = kptr[i].highPrice;                                                      \
            low[i] = kptr[i].lowPrice;                                                        \
        }                                                                                     \
                                                                                              \
        m_discard = back;                                                                     \
        auto *dst0 = this->data(0);                                                           \
        auto *dst1 = this->data(1);                                                           \
        int outBegIdx;                                                                        \
        int outNbElement;                                                                     \
        func(m_discard, total - 1, high, low, n, &outBegIdx, &outNbElement, dst0 + m_discard, \
             dst1 + m_discard);                                                               \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);          \
    }                                                                                         \
                                                                                              \
    Indicator HKU_API func(int n) {                                                           \
        auto p = make_shared<Cls_##func>();                                                   \
        p->setParam<int>("n", n);                                                             \
        p->calculate();                                                                       \
        return Indicator(p);                                                                  \
    }                                                                                         \
                                                                                              \
    Indicator HKU_API func(const KData &k, int n) {                                           \
        return Indicator(make_shared<Cls_##func>(k, n));                                      \
    }

#define TA_HLC_OUT3_N_IMP(func, func_lookback, period, period_min, period_max)          \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 3) {                                 \
        setParam<int>("n", period);                                                     \
    }                                                                                   \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 3) {            \
        setParam<KData>("kdata", k);                                                    \
        setParam<int>("n", n);                                                          \
        Cls_##func::_calculate(Indicator());                                            \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_checkParam(const string &name) const {                            \
        if (name == "n") {                                                              \
            int n = getParam<int>("n");                                                 \
            HKU_ASSERT(n >= period_min && n <= period_max);                             \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    void Cls_##func::_calculate(const Indicator &data) {                                \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                         \
                    "The input is ignored because {} depends on the context!", m_name); \
                                                                                        \
        KData k = getContext();                                                         \
        size_t total = k.size();                                                        \
        HKU_IF_RETURN(total == 0, void());                                              \
                                                                                        \
        _readyBuffer(total, 3);                                                         \
        int n = getParam<int>("n");                                                     \
        int back = func_lookback(n);                                                    \
        if (back < 0 || back >= total) {                                                \
            m_discard = total;                                                          \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        double *close = low + total;                                                    \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
            close[i] = kptr[i].closePrice;                                              \
        }                                                                               \
                                                                                        \
        auto *dst0 = this->data(0);                                                     \
        auto *dst1 = this->data(1);                                                     \
        auto *dst2 = this->data(2);                                                     \
        m_discard = back;                                                               \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(m_discard, total - 1, high, low, close, n, &outBegIdx, &outNbElement,      \
             dst0 + m_discard, dst1 + m_discard, dst2 + m_discard);                     \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);    \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(int n) {                                                     \
        auto p = make_shared<Cls_##func>();                                             \
        p->setParam<int>("n", n);                                                       \
        p->calculate();                                                                 \
        return Indicator(p);                                                            \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k, int n) {                                     \
        return Indicator(make_shared<Cls_##func>(k, n));                                \
    }

#define TA_OC_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)                   \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                         \
        setParam<int>("n", period);                                                             \
    }                                                                                           \
                                                                                                \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 1) {                    \
        setParam<KData>("kdata", k);                                                            \
        setParam<int>("n", n);                                                                  \
        Cls_##func::_calculate(Indicator());                                                    \
    }                                                                                           \
                                                                                                \
    void Cls_##func::_checkParam(const string &name) const {                                    \
        if (name == "n") {                                                                      \
            int n = getParam<int>("n");                                                         \
            HKU_ASSERT(n >= period_min && n <= period_max);                                     \
        }                                                                                       \
    }                                                                                           \
                                                                                                \
    void Cls_##func::_calculate(const Indicator &data) {                                        \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                                 \
                    "The input is ignored because {} depends on the context!", m_name);         \
                                                                                                \
        KData k = getContext();                                                                 \
        size_t total = k.size();                                                                \
        HKU_IF_RETURN(total == 0, void());                                                      \
                                                                                                \
        _readyBuffer(total, 1);                                                                 \
        int n = getParam<int>("n");                                                             \
        int back = func_lookback(n);                                                            \
        if (back < 0 || back >= total) {                                                        \
            m_discard = total;                                                                  \
            return;                                                                             \
        }                                                                                       \
                                                                                                \
        const KRecord *kptr = k.data();                                                         \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);                  \
        double *open = buf.get();                                                               \
        double *close = open + total;                                                           \
        for (size_t i = 0; i < total; ++i) {                                                    \
            open[i] = kptr[i].openPrice;                                                        \
            close[i] = kptr[i].closePrice;                                                      \
        }                                                                                       \
                                                                                                \
        auto *dst = this->data();                                                               \
        m_discard = back;                                                                       \
        int outBegIdx;                                                                          \
        int outNbElement;                                                                       \
        func(m_discard, total - 1, open, close, n, &outBegIdx, &outNbElement, dst + m_discard); \
        HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);            \
    }                                                                                           \
                                                                                                \
    Indicator HKU_API func(int n) {                                                             \
        auto p = make_shared<Cls_##func>();                                                     \
        p->setParam<int>("n", n);                                                               \
        p->calculate();                                                                         \
        return Indicator(p);                                                                    \
    }                                                                                           \
                                                                                                \
    Indicator HKU_API func(const KData &k, int n) {                                             \
        return Indicator(make_shared<Cls_##func>(k, n));                                        \
    }
