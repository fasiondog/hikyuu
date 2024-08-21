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
    string market;            ///< 市场标识
    string code;              ///< 证券代码
    string name;              ///< 证券名称
    Datetime datetime;        ///< 数据时间
    price_t yesterday_close;  ///< 昨日收盘价
    price_t open;             ///< 开盘价
    price_t high;             ///< 最高价
    price_t low;              ///< 最低价
    price_t close;            ///< 收盘价
    price_t amount;           ///< 成交金额 （千元）
    price_t volume;           ///< 成交量（手）
    price_t bid1;             ///< 买一价
    price_t bid1_amount;      ///< 买一数量（手）
    price_t bid2;             ///< 买二价
    price_t bid2_amount;      ///< 买二数量
    price_t bid3;             ///< 买三价
    price_t bid3_amount;      ///< 买三数量
    price_t bid4;             ///< 买四价
    price_t bid4_amount;      ///< 买四数量
    price_t bid5;             ///< 买五价
    price_t bid5_amount;      ///< 买五数量
    price_t ask1;             ///< 卖一价
    price_t ask1_amount;      ///< 卖一数量
    price_t ask2;             ///< 卖二价
    price_t ask2_amount;      ///< 卖二数量
    price_t ask3;             ///< 卖三价
    price_t ask3_amount;      ///< 卖三数量
    price_t ask4;             ///< 卖四价
    price_t ask4_amount;      ///< 卖四数量
    price_t ask5;             ///< 卖五价
    price_t ask5_amount;      ///< 卖五数量
};

}  // namespace hku