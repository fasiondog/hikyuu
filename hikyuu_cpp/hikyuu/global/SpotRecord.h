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
 * 接收外部实时数据结构
 * @ingroup Agent
 */
struct HKU_API SpotRecord {
    string market;              ///< 市场标识
    string code;                ///< 证券代码
    string name;                ///< 证券名称
    Datetime datetime;          ///< 数据时间
    price_t yesterday_close;    ///< 昨日收盘价
    price_t open;               ///< 开盘价
    price_t high;               ///< 最高价
    price_t low;                ///< 最低价
    price_t close;              ///< 收盘价
    price_t amount;             ///< 成交金额 （千元）
    price_t volume;             ///< 成交量（手）
    vector<double> bid;         ///< 委买价
    vector<double> bid_amount;  ///< 委买量
    vector<double> ask;         ///< 委卖价
    vector<double> ask_amount;  ///< 委卖量
};

}  // namespace hku