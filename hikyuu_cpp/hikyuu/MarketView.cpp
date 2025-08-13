/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/KDATA.h"
#include "hikyuu/indicator/crt/FINANCE.h"
#include "hikyuu/indicator/crt/HSL.h"
#include "hikyuu/indicator/crt/ZONGGUBEN.h"
#include "hikyuu/indicator/crt/LIUTONGPAN.h"
#include "StockManager.h"
#include "MarketView.h"

namespace hku {

MarketView HKU_API getMarketView(const StockList& stks, const string& market) {
    MarketView ret;
    DatetimeList dates = StockManager::instance().getTradingCalendar(KQueryByIndex(-2), market);
    HKU_IF_RETURN(dates.size() < 2, ret);  // 小于两个交易日暂时不处理

    Datetime yesterday = dates[0];
    Datetime today = dates[1];
    Datetime tomorrow = today.nextDay();

    Indicator EPS = FINANCE(0);
    EPS.setParam("dynamic", true);
    Indicator PE = CLOSE() / EPS;
    Indicator PB = CLOSE() / FINANCE(3);  // 市净率

    size_t total = stks.size();
    ret.resize(stks.size());
    size_t idx = 0;
    for (size_t i = 0; i < total; i++) {
        const Stock& stk = stks[i];
        if (!stk.isNull()) {
            auto kdata = stk.getKData(KQueryByDate(yesterday, tomorrow));
            if (kdata.size() == 2) {
                auto& mr = ret[idx++];
                mr.stock = stk;
                mr.date = kdata[1].datetime;
                mr.open = kdata[1].openPrice;
                mr.high = kdata[1].highPrice;
                mr.low = kdata[1].lowPrice;
                mr.close = kdata[1].closePrice;
                mr.amount = kdata[1].transAmount;
                mr.volume = kdata[1].transCount;
                mr.yesterday_close = kdata[0].closePrice;
                Indicator::value_t hsl = HSL(kdata)[1];
                if (hsl != Null<Indicator::value_t>()) {
                    mr.turnover = hsl * 100.0;
                }
                mr.amplitude =
                  mr.open != 0.0 ? ((mr.high - mr.low) / mr.open) * 100.0 : Null<price_t>();
                mr.price_change = mr.yesterday_close != 0.0
                                    ? ((mr.close - mr.yesterday_close) / mr.yesterday_close) * 100.0
                                    : Null<price_t>();
                Indicator::value_t zongguben = ZONGGUBEN(kdata)[1];
                if (zongguben != Null<Indicator::value_t>()) {
                    mr.total_market_cap = zongguben * mr.close;
                }
                Indicator::value_t liutongpan = LIUTONGPAN(kdata)[1];
                if (liutongpan != Null<Indicator::value_t>()) {
                    mr.circulating_market_cap = liutongpan * mr.close;
                }
                Indicator::value_t pe = PE(kdata)[1];
                if (pe != Null<Indicator::value_t>()) {
                    mr.pe = pe;
                }
                Indicator::value_t pb = PB(kdata)[1];
                if (pb != Null<Indicator::value_t>()) {
                    mr.pb = pb;
                }
            } else if (kdata.size() == 1 && kdata[0].datetime == dates[1]) {
                auto& mr = ret[idx++];
                mr.stock = stk;
                mr.date = kdata[0].datetime;
                mr.open = kdata[0].openPrice;
                mr.high = kdata[0].highPrice;
                mr.low = kdata[0].lowPrice;
                mr.close = kdata[0].closePrice;
                mr.amount = kdata[0].transAmount;
                mr.volume = kdata[0].transCount;
                mr.yesterday_close = Null<price_t>();
                Indicator::value_t hsl = HSL(kdata)[0];
                if (hsl != Null<Indicator::value_t>()) {
                    mr.turnover = hsl * 100.0;
                }
                mr.amplitude =
                  mr.open != 0.0 ? ((mr.high - mr.low) / mr.open) * 100.0 : Null<price_t>();
                mr.price_change =
                  mr.open != 0.0 ? ((mr.close - mr.open) / mr.open) * 100.0 : Null<price_t>();
                Indicator::value_t zongguben = ZONGGUBEN(kdata)[0];
                if (zongguben != Null<Indicator::value_t>()) {
                    mr.total_market_cap = zongguben * mr.close;
                }
                Indicator::value_t liutongpan = LIUTONGPAN(kdata)[0];
                if (liutongpan != Null<Indicator::value_t>()) {
                    mr.circulating_market_cap = liutongpan * mr.close;
                }
                Indicator::value_t pe = PE(kdata)[0];
                if (pe != Null<Indicator::value_t>()) {
                    mr.pe = pe;
                }
                Indicator::value_t pb = PB(kdata)[0];
                if (pb != Null<Indicator::value_t>()) {
                    mr.pb = pb;
                }
            }
        }
    }
    ret.resize(idx);
    return ret;
}

}  // namespace hku