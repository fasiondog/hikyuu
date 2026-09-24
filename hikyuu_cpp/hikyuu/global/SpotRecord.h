/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

/**
 * Data structure of the received external realtime data
 * @ingroup Agent
 */
struct HKU_API SpotRecord {
    string market;              ///< Market identifier
    string code;                ///< Security code
    string name;                ///< Security name
    Datetime datetime;          ///< Data time
    price_t yesterday_close;    ///< Yesterday close price
    price_t open;               ///< Open price
    price_t high;               ///< High price
    price_t low;                ///< Low price
    price_t close;              ///< Close price
    price_t amount;             ///< Turnover amount (in units of 1 thousand yuan)
    price_t volume;             ///< Trading volume (in lots)
    vector<double> bid;         ///< Bid price
    vector<double> bid_amount;  ///< Bid quantity
    vector<double> ask;         ///< Ask price
    vector<double> ask_amount;  ///< Ask quantity
};

}  // namespace hku