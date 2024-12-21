/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#define EXPOERT_TA_FUNC(func) BOOST_CLASS_EXPORT(hku::Cls_##func)

#define TA_IN1_OUT1_IMP(func)                                                              \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                                   \
    Cls_##func::~Cls_##func() {}                                                           \
                                                                                           \
    void Cls_##func::_checkParam(const string &name) const {}                              \
                                                                                           \
    void Cls_##func::_calculate(const Indicator &data) {                                   \
        m_discard = data.discard();                                                        \
        size_t total = data.size();                                                        \
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
        func(m_discard, total - 1, src, &outBegIdx, &outNbElement, dst + m_discard);       \
        if (outBegIdx != m_discard) {                                                      \
            memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);      \
            double null_double = Null<double>();                                           \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                               \
                _set(null_double, i);                                                      \
            }                                                                              \
            m_discard = outBegIdx;                                                         \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) { \
        HKU_IF_RETURN(ind.discard() > curPos, void());                                     \
        double buf[1];                                                                     \
        auto const *src = ind.data();                                                      \
        int outBegIdx;                                                                     \
        int outNbElement;                                                                  \
        func(curPos, curPos + 1, src, &outBegIdx, &outNbElement, buf);                     \
        if (outNbElement >= 1) {                                                           \
            _set(buf[0], curPos);                                                          \
        }                                                                                  \
    }                                                                                      \
                                                                                           \
    Indicator HKU_API func() {                                                             \
        return Indicator(make_shared<Cls_##func>());                                       \
    }

#define TA_IN1_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)               \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                      \
        setParam<int>("n", period);                                                          \
    }                                                                                        \
    Cls_##func::~Cls_##func() {}                                                             \
                                                                                             \
    void Cls_##func::_checkParam(const string &name) const {                                 \
        if (name == "n") {                                                                   \
            int n = getParam<int>("n");                                                      \
            HKU_ASSERT(n >= period_min && n <= period_max);                                  \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_calculate(const Indicator &data) {                                     \
        int n = getParam<int>("n");                                                          \
        m_discard = data.discard() + func_lookback(n);                                       \
        size_t total = data.size();                                                          \
        if (m_discard >= total) {                                                            \
            m_discard = total;                                                               \
            return;                                                                          \
        }                                                                                    \
                                                                                             \
        auto const *src = data.data();                                                       \
        auto *dst = this->data();                                                            \
                                                                                             \
        int outBegIdx;                                                                       \
        int outNbElement;                                                                    \
        func(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement, dst + m_discard); \
        if (outBegIdx != m_discard) {                                                        \
            memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);        \
            double null_double = Null<double>();                                             \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                                 \
                _set(null_double, i);                                                        \
            }                                                                                \
            m_discard = outBegIdx;                                                           \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) {   \
        int back = func_lookback(step);                                                      \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                       \
                                                                                             \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(curPos);                  \
        auto const *src = ind.data();                                                        \
        int outBegIdx;                                                                       \
        int outNbElement;                                                                    \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, buf.get());        \
        if (outNbElement >= 1) {                                                             \
            _set(buf.get()[outNbElement - 1], curPos);                                       \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(int n) {                                                          \
        auto p = make_shared<Cls_##func>();                                                  \
        p->setParam<int>("n", n);                                                            \
        return Indicator(p);                                                                 \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(const IndParam &n) {                                              \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                       \
        p->setIndParam("n", n);                                                              \
        return Indicator(p);                                                                 \
    }

#define TA_IN1_OUT2_N_IMP(func, func_lookback, period, period_min, period_max)               \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                                      \
        setParam<int>("n", period);                                                          \
    }                                                                                        \
    Cls_##func::~Cls_##func() {}                                                             \
                                                                                             \
    void Cls_##func::_checkParam(const string &name) const {                                 \
        if (name == "n") {                                                                   \
            int n = getParam<int>("n");                                                      \
            HKU_ASSERT(n >= period_min && n <= period_max);                                  \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_calculate(const Indicator &data) {                                     \
        int n = getParam<int>("n");                                                          \
        m_discard = data.discard() + func_lookback(n);                                       \
        size_t total = data.size();                                                          \
        if (m_discard >= total) {                                                            \
            m_discard = total;                                                               \
            return;                                                                          \
        }                                                                                    \
                                                                                             \
        auto const *src = data.data();                                                       \
        auto *dst0 = this->data(0);                                                          \
        auto *dst1 = this->data(1);                                                          \
                                                                                             \
        int outBegIdx;                                                                       \
        int outNbElement;                                                                    \
        func(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement, dst0 + m_discard, \
             dst1 + m_discard);                                                              \
        if (outBegIdx != m_discard) {                                                        \
            memmove(dst0 + outBegIdx, dst0 + m_discard, sizeof(double) * outNbElement);      \
            memmove(dst1 + outBegIdx, dst1 + m_discard, sizeof(double) * outNbElement);      \
            double null_double = Null<double>();                                             \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                                 \
                _set(null_double, i, 0);                                                     \
                _set(null_double, i, 1);                                                     \
            }                                                                                \
            m_discard = outBegIdx;                                                           \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, size_t step) {   \
        int back = func_lookback(step);                                                      \
        HKU_IF_RETURN(back<0 || back + ind.discard()> curPos, void());                       \
                                                                                             \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * curPos);              \
        double *dst0 = buf.get();                                                            \
        double *ds1 = dst0 + curPos;                                                         \
        auto const *src = ind.data();                                                        \
        int outBegIdx;                                                                       \
        int outNbElement;                                                                    \
        func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, dst0, ds1);        \
        if (outNbElement >= 1) {                                                             \
            _set(dst0[outNbElement - 1], curPos, 0);                                         \
            _set(ds1[outNbElement - 1], curPos, 1);                                          \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(int n) {                                                          \
        auto p = make_shared<Cls_##func>();                                                  \
        p->setParam<int>("n", n);                                                            \
        return Indicator(p);                                                                 \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(const IndParam &n) {                                              \
        IndicatorImpPtr p = make_shared<Cls_##func>();                                       \
        p->setIndParam("n", n);                                                              \
        return Indicator(p);                                                                 \
    }

#define TA_OHLC_OUT1_IMP(func)                                                          \
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
        if (total == 0) {                                                               \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
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
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(0, total - 1, open, high, low, close, &outBegIdx, &outNbElement, dst);     \
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
        m_discard = func_lookback();                                                    \
        HKU_IF_RETURN(m_discard >= total, void());                                      \
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
        std::unique_ptr<int[]> outbuf = std::make_unique<int[]>(total - m_discard);     \
                                                                                        \
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(m_discard, total - 1, open, high, low, close, &outBegIdx, &outNbElement,   \
             outbuf.get() + m_discard);                                                 \
        m_discard = outBegIdx;                                                          \
        for (size_t i = m_discard; i < total; ++i) {                                    \
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

#define TA_HLCV_OUT1_IMP(func)                                                          \
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
        if (total == 0) {                                                               \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
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
        func(0, total - 1, high, low, close, vol, &outBegIdx, &outNbElement, dst);      \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func() {                                                          \
        return make_shared<Cls_##func>()->calculate();                                  \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k) {                                            \
        return Indicator(make_shared<Cls_##func>(k));                                   \
    }

#define TA_HL_OUT1_IMP(func)                                                            \
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
        if (total == 0) {                                                               \
            return;                                                                     \
        }                                                                               \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
                                                                                        \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
        }                                                                               \
                                                                                        \
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(0, total - 1, high, low, &outBegIdx, &outNbElement, dst);                  \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func() {                                                          \
        return make_shared<Cls_##func>()->calculate();                                  \
    }                                                                                   \
                                                                                        \
    Indicator HKU_API func(const KData &k) {                                            \
        return Indicator(make_shared<Cls_##func>(k));                                   \
    }

#define TA_HLC_OUT1_IMP(func)                                                           \
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
        auto *dst = this->data();                                                       \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(0, total - 1, high, low, close, &outBegIdx, &outNbElement, dst);           \
        if (outBegIdx > 0) {                                                            \
            memmove(dst + outBegIdx, dst, sizeof(double) * outNbElement);               \
            double null_double = Null<double>();                                        \
            for (size_t i = 0; i < outBegIdx; ++i) {                                    \
                _set(null_double, i);                                                   \
            }                                                                           \
            m_discard = outBegIdx;                                                      \
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

#define TA_HLC_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)               \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                      \
        setParam<int>("n", period);                                                          \
    }                                                                                        \
                                                                                             \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 1) {                 \
        setParam<KData>("kdata", k);                                                         \
        setParam<int>("n", n);                                                               \
        Cls_##func::_calculate(Indicator());                                                 \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_checkParam(const string &name) const {                                 \
        if (name == "n") {                                                                   \
            int n = getParam<int>("n");                                                      \
            HKU_ASSERT(n >= period_min && n <= period_max);                                  \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_calculate(const Indicator &data) {                                     \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                              \
                    "The input is ignored because {} depends on the context!", m_name);      \
                                                                                             \
        KData k = getContext();                                                              \
        size_t total = k.size();                                                             \
        HKU_IF_RETURN(total == 0, void());                                                   \
                                                                                             \
        int n = getParam<int>("n");                                                          \
        int back = func_lookback(n);                                                         \
        HKU_IF_RETURN(back < 0, void());                                                     \
                                                                                             \
        _readyBuffer(total, 1);                                                              \
        const KRecord *kptr = k.data();                                                      \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);               \
        double *high = buf.get();                                                            \
        double *low = high + total;                                                          \
        double *close = low + total;                                                         \
        for (size_t i = 0; i < total; ++i) {                                                 \
            high[i] = kptr[i].highPrice;                                                     \
            low[i] = kptr[i].lowPrice;                                                       \
            close[i] = kptr[i].closePrice;                                                   \
        }                                                                                    \
                                                                                             \
        auto *dst = this->data();                                                            \
        m_discard = back;                                                                    \
        int outBegIdx;                                                                       \
        int outNbElement;                                                                    \
        func(0, total - 1, high, low, close, n, &outBegIdx, &outNbElement, dst + m_discard); \
        if (outBegIdx != m_discard) {                                                        \
            memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);        \
            double null_double = Null<double>();                                             \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                                 \
                _set(null_double, i);                                                        \
            }                                                                                \
            m_discard = outBegIdx;                                                           \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(int n) {                                                          \
        auto p = make_shared<Cls_##func>();                                                  \
        p->setParam<int>("n", n);                                                            \
        p->calculate();                                                                      \
        return Indicator(p);                                                                 \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(const KData &k, int n) {                                          \
        return Indicator(make_shared<Cls_##func>(k, n));                                     \
    }

#define TA_HLCV_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)                   \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                                           \
        setParam<int>("n", period);                                                               \
    }                                                                                             \
                                                                                                  \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 1) {                      \
        setParam<KData>("kdata", k);                                                              \
        setParam<int>("n", n);                                                                    \
        Cls_##func::_calculate(Indicator());                                                      \
    }                                                                                             \
                                                                                                  \
    void Cls_##func::_checkParam(const string &name) const {                                      \
        if (name == "n") {                                                                        \
            int n = getParam<int>("n");                                                           \
            HKU_ASSERT(n >= period_min && n <= period_max);                                       \
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
        int n = getParam<int>("n");                                                               \
        int back = func_lookback(n);                                                              \
        HKU_IF_RETURN(back < 0, void());                                                          \
                                                                                                  \
        _readyBuffer(total, 1);                                                                   \
        const KRecord *kptr = k.data();                                                           \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);                    \
        double *high = buf.get();                                                                 \
        double *low = high + total;                                                               \
        double *close = low + total;                                                              \
        double *vol = close + total;                                                              \
        for (size_t i = 0; i < total; ++i) {                                                      \
            high[i] = kptr[i].highPrice;                                                          \
            low[i] = kptr[i].lowPrice;                                                            \
            close[i] = kptr[i].closePrice;                                                        \
            vol[i] = kptr[i].transCount;                                                          \
        }                                                                                         \
                                                                                                  \
        auto *dst = this->data();                                                                 \
        m_discard = back;                                                                         \
        int outBegIdx;                                                                            \
        int outNbElement;                                                                         \
        func(0, total - 1, high, low, close, vol, n, &outBegIdx, &outNbElement, dst + m_discard); \
        if (outBegIdx != m_discard) {                                                             \
            memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);             \
            double null_double = Null<double>();                                                  \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                                      \
                _set(null_double, i);                                                             \
            }                                                                                     \
            m_discard = outBegIdx;                                                                \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(int n) {                                                               \
        auto p = make_shared<Cls_##func>();                                                       \
        p->setParam<int>("n", n);                                                                 \
        p->calculate();                                                                           \
        return Indicator(p);                                                                      \
    }                                                                                             \
                                                                                                  \
    Indicator HKU_API func(const KData &k, int n) {                                               \
        return Indicator(make_shared<Cls_##func>(k, n));                                          \
    }

#define TA_HL_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)           \
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
        int n = getParam<int>("n");                                                     \
        int back = func_lookback(n);                                                    \
        HKU_IF_RETURN(back < 0, void());                                                \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
        }                                                                               \
                                                                                        \
        auto *dst = this->data();                                                       \
        m_discard = back;                                                               \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(0, total - 1, high, low, n, &outBegIdx, &outNbElement, dst + m_discard);   \
        if (outBegIdx != m_discard) {                                                   \
            memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);   \
            double null_double = Null<double>();                                        \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                            \
                _set(null_double, i);                                                   \
            }                                                                           \
            m_discard = outBegIdx;                                                      \
        }                                                                               \
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

#define TA_HL_OUT2_N_IMP(func, func_lookback, period, period_min, period_max)           \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                                 \
        setParam<int>("n", period);                                                     \
    }                                                                                   \
                                                                                        \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 2) {            \
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
        int n = getParam<int>("n");                                                     \
        int back = func_lookback(n);                                                    \
        HKU_IF_RETURN(back < 0, void());                                                \
                                                                                        \
        _readyBuffer(total, 2);                                                         \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);          \
        double *high = buf.get();                                                       \
        double *low = high + total;                                                     \
        for (size_t i = 0; i < total; ++i) {                                            \
            high[i] = kptr[i].highPrice;                                                \
            low[i] = kptr[i].lowPrice;                                                  \
        }                                                                               \
                                                                                        \
        auto *dst0 = this->data(0);                                                     \
        auto *dst1 = this->data(1);                                                     \
        m_discard = back;                                                               \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(0, total - 1, high, low, n, &outBegIdx, &outNbElement, dst0 + m_discard,   \
             dst1 + m_discard);                                                         \
        if (outBegIdx != m_discard) {                                                   \
            memmove(dst0 + outBegIdx, dst0 + m_discard, sizeof(double) * outNbElement); \
            memmove(dst1 + outBegIdx, dst1 + m_discard, sizeof(double) * outNbElement); \
            double null_double = Null<double>();                                        \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                            \
                _set(null_double, i, 0);                                                \
                _set(null_double, i, 1);                                                \
            }                                                                           \
            m_discard = outBegIdx;                                                      \
        }                                                                               \
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

#define TA_HLC_OUT3_N_IMP(func, func_lookback, period, period_min, period_max)               \
    Cls_##func::Cls_##func() : IndicatorImp(#func, 3) {                                      \
        setParam<int>("n", period);                                                          \
    }                                                                                        \
                                                                                             \
    Cls_##func::Cls_##func(const KData &k, int n) : IndicatorImp(#func, 3) {                 \
        setParam<KData>("kdata", k);                                                         \
        setParam<int>("n", n);                                                               \
        Cls_##func::_calculate(Indicator());                                                 \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_checkParam(const string &name) const {                                 \
        if (name == "n") {                                                                   \
            int n = getParam<int>("n");                                                      \
            HKU_ASSERT(n >= period_min && n <= period_max);                                  \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    void Cls_##func::_calculate(const Indicator &data) {                                     \
        HKU_WARN_IF(!isLeaf() && !data.empty(),                                              \
                    "The input is ignored because {} depends on the context!", m_name);      \
                                                                                             \
        KData k = getContext();                                                              \
        size_t total = k.size();                                                             \
        HKU_IF_RETURN(total == 0, void());                                                   \
                                                                                             \
        int n = getParam<int>("n");                                                          \
        int back = func_lookback(n);                                                         \
        HKU_IF_RETURN(back < 0, void());                                                     \
                                                                                             \
        _readyBuffer(total, 3);                                                              \
        const KRecord *kptr = k.data();                                                      \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);               \
        double *high = buf.get();                                                            \
        double *low = high + total;                                                          \
        double *close = low + total;                                                         \
        for (size_t i = 0; i < total; ++i) {                                                 \
            high[i] = kptr[i].highPrice;                                                     \
            low[i] = kptr[i].lowPrice;                                                       \
            close[i] = kptr[i].closePrice;                                                   \
        }                                                                                    \
                                                                                             \
        auto *dst0 = this->data(0);                                                          \
        auto *dst1 = this->data(1);                                                          \
        auto *dst2 = this->data(2);                                                          \
        m_discard = back;                                                                    \
        int outBegIdx;                                                                       \
        int outNbElement;                                                                    \
        func(0, total - 1, high, low, close, n, &outBegIdx, &outNbElement, dst0 + m_discard, \
             dst1 + m_discard, dst2 + m_discard);                                            \
        if (outBegIdx != m_discard) {                                                        \
            memmove(dst0 + outBegIdx, dst0 + m_discard, sizeof(double) * outNbElement);      \
            memmove(dst1 + outBegIdx, dst1 + m_discard, sizeof(double) * outNbElement);      \
            memmove(dst2 + outBegIdx, dst2 + m_discard, sizeof(double) * outNbElement);      \
            double null_double = Null<double>();                                             \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                                 \
                _set(null_double, i, 0);                                                     \
                _set(null_double, i, 1);                                                     \
                _set(null_double, i, 2);                                                     \
            }                                                                                \
            m_discard = outBegIdx;                                                           \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(int n) {                                                          \
        auto p = make_shared<Cls_##func>();                                                  \
        p->setParam<int>("n", n);                                                            \
        p->calculate();                                                                      \
        return Indicator(p);                                                                 \
    }                                                                                        \
                                                                                             \
    Indicator HKU_API func(const KData &k, int n) {                                          \
        return Indicator(make_shared<Cls_##func>(k, n));                                     \
    }

#define TA_OC_OUT1_N_IMP(func, func_lookback, period, period_min, period_max)           \
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
        int n = getParam<int>("n");                                                     \
        int back = func_lookback(n);                                                    \
        HKU_IF_RETURN(back < 0, void());                                                \
                                                                                        \
        _readyBuffer(total, 1);                                                         \
        const KRecord *kptr = k.data();                                                 \
        std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);          \
        double *open = buf.get();                                                       \
        double *close = open + total;                                                   \
        for (size_t i = 0; i < total; ++i) {                                            \
            open[i] = kptr[i].openPrice;                                                \
            close[i] = kptr[i].closePrice;                                              \
        }                                                                               \
                                                                                        \
        auto *dst = this->data();                                                       \
        m_discard = back;                                                               \
        int outBegIdx;                                                                  \
        int outNbElement;                                                               \
        func(0, total - 1, open, close, n, &outBegIdx, &outNbElement, dst + m_discard); \
        if (outBegIdx != m_discard) {                                                   \
            memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);   \
            double null_double = Null<double>();                                        \
            for (size_t i = m_discard; i < outBegIdx; ++i) {                            \
                _set(null_double, i);                                                   \
            }                                                                           \
            m_discard = outBegIdx;                                                      \
        }                                                                               \
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