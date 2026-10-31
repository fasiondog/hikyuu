/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include <climits>
#include "IRSRSBeta.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRSRSBeta)
#endif

namespace hku {

IRSRSBeta::IRSRSBeta() : IndicatorImp("RSRS_BETA", 1) {
    m_need_context = true;
    setParam<int>("n", 20);
}

IRSRSBeta::~IRSRSBeta() {}

void IRSRSBeta::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2 && getParam<int>("n") < INT_MAX);
    }
}

void IRSRSBeta::_calculate(const Indicator&) {
    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    int n = getParam<int>("n");
    if (total < static_cast<size_t>(n)) {
        m_discard = total;
        return;
    }

    auto* dst = this->data();
    m_discard = n - 1;
    value_t null_price = Null<price_t>();

    // Centered accumulation with a valid_count filter (ST-034/ST-N03 contract): non-finite
    // low/high pairs are excluded from the fit; the anchor is the first bar's pair, sanitized to
    // 0.0 when non-finite (the slope is shift-invariant, so any finite anchor gives the same
    // result)
    value_t kx = k[0].lowPrice;
    value_t ky = k[0].highPrice;
    if (!std::isfinite(kx))
        kx = 0.0;
    if (!std::isfinite(ky))
        ky = 0.0;

    value_t ex = 0.0, ey = 0.0, exx = 0.0, exy = 0.0;
    size_t valid = 0;
    for (size_t i = 0; i < static_cast<size_t>(n); i++) {
        value_t x = k[i].lowPrice;
        value_t y = k[i].highPrice;
        if (std::isfinite(x) && std::isfinite(y)) {
            value_t ix = x - kx;
            value_t iy = y - ky;
            ex += ix;
            ey += iy;
            exx += ix * ix;
            exy += ix * iy;
            valid++;
        }
    }

    auto emit = [&](size_t i) {
        if (valid < 2)
            return;
        value_t nv = (value_t)valid;
        value_t varx = exx - ex * ex / nv;
        value_t cov = exy - ex * ey / nv;
        dst[i] =
          !(varx > 0.0 && std::isfinite(varx) && std::isfinite(cov)) ? null_price : cov / varx;
    };
    emit(n - 1);

    for (size_t i = n; i < total; i++) {
        value_t old_x = k[i - n].lowPrice;
        value_t old_y = k[i - n].highPrice;
        if (std::isfinite(old_x) && std::isfinite(old_y)) {
            value_t pix = old_x - kx;
            value_t piy = old_y - ky;
            ex -= pix;
            ey -= piy;
            exx -= pix * pix;
            exy -= pix * piy;
            valid--;
        }
        value_t new_x = k[i].lowPrice;
        value_t new_y = k[i].highPrice;
        if (std::isfinite(new_x) && std::isfinite(new_y)) {
            value_t pix = new_x - kx;
            value_t piy = new_y - ky;
            ex += pix;
            ey += piy;
            exx += pix * pix;
            exy += pix * piy;
            valid++;
        }
        emit(i);
    }
}

Indicator HKU_API RSRS_BETA(int n) {
    auto p = make_shared<IRSRSBeta>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API RSRS_BETA(const KData& kdata, int n) {
    auto p = make_shared<IRSRSBeta>();
    p->setParam<int>("n", n);
    p->setContext(kdata);
    return Indicator(p);
}

}  // namespace hku
