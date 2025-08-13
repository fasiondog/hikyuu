/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#pragma once

#include "Stock.h"

namespace hku {

/**
 * 实时行情记录(用于显示打印)
 * @ingroup StockManage
 */
struct HKU_API MarketViewRecord {
    Stock stock;                                      ///< 证券
    Datetime date;                                    ///< 日期
    price_t open{Null<price_t>()};                    ///< 开盘价
    price_t high{Null<price_t>()};                    ///< 最高价
    price_t low{Null<price_t>()};                     ///< 最低价
    price_t close{Null<price_t>()};                   ///< 收盘价
    price_t amount{Null<price_t>()};                  ///< 成交额
    price_t volume{Null<price_t>()};                  ///< 成交量
    price_t yesterday_close{Null<price_t>()};         ///< 昨收价
    price_t turnover{Null<price_t>()};                ///< 换手率(%)
    price_t amplitude{Null<price_t>()};               ///< 振幅(%)
    price_t price_change{Null<price_t>()};            ///< 涨跌幅(%)
    price_t total_market_cap{Null<price_t>()};        ///< 总市值
    price_t circulating_market_cap{Null<price_t>()};  ///< 流通市值
    price_t pe{Null<price_t>()};                      ///< 动态市盈率
    price_t pb{Null<price_t>()};                      ///< 市净率
};

using MarketView = vector<MarketViewRecord>;

/**
 * @brief 获取指定股票的行情数据（最后交易日数据），不包含停牌无最后交易日的股票数据
 * @ingroup StockManage
 * @param stks 股票列表
 * @param market 市场代码（用于获取交易日历）
 * @return MarketView
 */
MarketView HKU_API getMarketView(const StockList& stks, const string& market = "SH");

}  // namespace hku
