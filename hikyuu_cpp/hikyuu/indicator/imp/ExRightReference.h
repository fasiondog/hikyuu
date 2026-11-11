/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/StockWeight.h"

namespace hku {

/**
 * @brief The ex-rights/ex-dividend reference price used by the exchange as the previous close
 * @details [(record date close - cash dividend) + rights issue price × share change ratio] /
 * (1 + share change ratio), where the share change ratio covers bonus, rights issue and
 * capitalized shares (per 10 shares / 10)
 */
inline price_t ex_right_reference_price(price_t record_close, const StockWeight& weight) {
    if (weight.suogu() != 0.0) {
        return record_close / weight.suogu();
    }
    const price_t change =
      0.1 * (weight.countAsGift() + weight.countForSell() + weight.increasement());
    return (record_close - 0.1 * weight.bonus() + weight.priceForSell() * change) / (1.0 + change);
}

}  // namespace hku
