/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#include "hikyuu/Stock.h"
#include "hikyuu/StockTypeInfo.h"
#include "ExRightReference.h"
#include "IIsLimitDown.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIsLimitDown)
#endif

namespace hku {

IIsLimitDown::IIsLimitDown() : IndicatorImp("ISLIMITDOWN", 1) {
    m_need_context = true;
}

IIsLimitDown::~IIsLimitDown() {}

void IIsLimitDown::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", name());

    size_t total = getContext().size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    m_discard = 1;  // No previous K-line, so the limit down cannot be judged; discard it
    _increment_calculate(ind, 1);
}

size_t IIsLimitDown::min_increment_start() const {
    // ks[i - 1] is read, so the first bar of the context cannot be judged
    return 1;
}

void IIsLimitDown::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    value_t limit_down = 0.0;
    const Stock& stock = kdata.getStock();
    if (stock.type() == STOCKTYPE_A) {
        limit_down =
          0.9;  // 10% for the A-shares, but 5% for the ST stocks (not handled: no ST date)
    } else if (stock.type() == STOCKTYPE_A_BJ) {
        limit_down = 0.7;  // 30% for the Beijing Stock Exchange
    } else if (stock.type() == STOCKTYPE_GEM || stock.type() == STOCKTYPE_START) {
        limit_down = 0.8;  // 20% for the ChiNext and the STAR Market
    }

    auto* dst = this->data();
    if (limit_down <= 0.0) {
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = 0.0;  // An unsupported security type, regarded as not limit down
        }
        return;
    }

    // The exchange judges the limit on the raw closes; a recovered context is re-fetched with the
    // same query, otherwise the prices are on the adjusted scale and cannot match the exchange one
    const KQuery& q = kdata.getQuery();
    bool raw_prices = q.recoverType() == KQuery::NO_RECOVER;
    auto const* ks = kdata.data();
    if (!raw_prices) {
        KQuery nq = q;
        nq.recoverType(KQuery::NO_RECOVER);
        KData raw = kdata.getKData(nq);
        if (raw.size() == total) {
            ks = raw.data();
            raw_prices = true;
        }
    }

    // Only the records covering the context matter: the exchange uses the reference price of the
    // current ex-rights day as the previous close, so the whole history is not needed
    StockWeightList weights;
    if (raw_prices && total > 0) {
        weights = stock.getWeight(ks[0].datetime.startOfDay(),
                                  ks[total - 1].datetime.startOfDay() + Days(1));
    }
    size_t weight_pos = 0;
    if (!weights.empty() && start_pos < total) {
        const Datetime first_day = ks[start_pos].datetime.startOfDay();
        while (weight_pos < weights.size() &&
               weights[weight_pos].datetime().startOfDay() < first_day) {
            ++weight_pos;
        }
    }

    for (size_t i = start_pos; i < total; ++i) {
        price_t prev_close = ks[i - 1].closePrice;
        if (!weights.empty()) {
            const Datetime day = ks[i].datetime.startOfDay();
            while (weight_pos < weights.size() &&
                   weights[weight_pos].datetime().startOfDay() < day) {
                ++weight_pos;
            }
            // Only the first bar of the ex-rights day takes the reference price as the previous
            // close (the intraday bars keep comparing with the previous bar)
            if (weight_pos < weights.size() && weights[weight_pos].datetime().startOfDay() == day &&
                ks[i - 1].datetime.startOfDay() < day) {
                const price_t ref = ex_right_reference_price(prev_close, weights[weight_pos]);
                if (ref > 0.0) {
                    prev_close = roundEx(ref, stock.precision());
                }
            }
        }
        dst[i] = value_t(ks[i].closePrice <= roundEx(prev_close * limit_down, stock.precision()));
    }
}

Indicator HKU_API ISLIMITDOWN() {
    return make_shared<IIsLimitDown>()->calculate();
}

Indicator HKU_API ISLIMITDOWN(const KData& k) {
    auto p = make_shared<IIsLimitDown>();
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku
