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
#include "hikyuu/StockManager.h"
#include "MarketView.h"

namespace hku {

std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks, const Datetime& date,
                                                    const string& market) {
    std::shared_ptr<arrow::Table> ret;
    auto& sm = StockManager::instance();
    MarketInfo info = sm.getMarketInfo(market);
    HKU_IF_RETURN(info.code().empty(), ret);

    Stock market_stk = sm.getStock(fmt::format("{}{}", market, info.code()));
    HKU_IF_RETURN(market_stk.isNull(), ret);

    size_t start_pos, end_pos;
    HKU_IF_RETURN(!market_stk.getIndexRange(KQueryByDate(date.startOfDay(), date.nextDay()),
                                            start_pos, end_pos),
                  ret);
    HKU_IF_RETURN(start_pos < 1, ret);

    DatetimeList dates = market_stk.getDatetimeList(KQueryByIndex(start_pos - 1, start_pos + 1));
    HKU_CHECK(dates.size() == 2, "Invalid index!");

    Datetime yesterday = dates[0];
    Datetime today = dates[1];
    Datetime tomorrow = today.nextDay();

    Indicator EPS = FINANCE(0);
    EPS.setParam("dynamic", true);
    Indicator PE = CLOSE() / EPS;
    Indicator PB = CLOSE() / FINANCE(3);  // 市净率

    arrow::StringBuilder code_builder, name_builder;
    arrow::Date64Builder date_builder;
    arrow::DoubleBuilder open_builder, high_builder, low_builder, close_builder, amount_builder,
      volume_builder, yesterday_close_builder, turnover_builder, amplitude_builder,
      price_change_builder, total_market_cap_builder, circulating_market_cap_builder, pe_builder,
      pb_builder;

    size_t total = stks.size();
    HKU_ERROR_IF_RETURN(!code_builder.Reserve(total).ok(), ret, "Failed to reserve code_builder!");
    HKU_ERROR_IF_RETURN(!name_builder.Reserve(total).ok(), ret, "Failed to reserve name_builder!");
    HKU_ERROR_IF_RETURN(!date_builder.Reserve(total).ok(), ret, "Failed to reserve date_builder!");
    HKU_ERROR_IF_RETURN(!open_builder.Reserve(total).ok(), ret, "Failed to reserve open_builder!");
    HKU_ERROR_IF_RETURN(!high_builder.Reserve(total).ok(), ret, "Failed to reserve high_builder!");
    HKU_ERROR_IF_RETURN(!low_builder.Reserve(total).ok(), ret, "Failed to reserve low_builder!");
    HKU_ERROR_IF_RETURN(!close_builder.Reserve(total).ok(), ret,
                        "Failed to reserve close_builder!");
    HKU_ERROR_IF_RETURN(!amount_builder.Reserve(total).ok(), ret,
                        "Failed to reserve amount_builder!");
    HKU_ERROR_IF_RETURN(!volume_builder.Reserve(total).ok(), ret,
                        "Failed to reserve volume_builder!");
    HKU_ERROR_IF_RETURN(!yesterday_close_builder.Reserve(total).ok(), ret,
                        "Failed to reserve yesterday_close_builder!");
    HKU_ERROR_IF_RETURN(!turnover_builder.Reserve(total).ok(), ret,
                        "Failed to reserve turnover_builder!");
    HKU_ERROR_IF_RETURN(!amplitude_builder.Reserve(total).ok(), ret,
                        "Failed to reserve amplitude_builder!");
    HKU_ERROR_IF_RETURN(!price_change_builder.Reserve(total).ok(), ret,
                        "Failed to reserve price_change_builder!");
    HKU_ERROR_IF_RETURN(!total_market_cap_builder.Reserve(total).ok(), ret,
                        "Failed to reserve total_market_cap_builder!");
    HKU_ERROR_IF_RETURN(!circulating_market_cap_builder.Reserve(total).ok(), ret,
                        "Failed to reserve circulating_market_cap_builder!");
    HKU_ERROR_IF_RETURN(!pe_builder.Reserve(total).ok(), ret, "Failed to reserve pe_builder!");
    HKU_ERROR_IF_RETURN(!pb_builder.Reserve(total).ok(), ret, "Failed to reserve pb_builder!");

    for (size_t i = 0; i < total; i++) {
        const Stock& stk = stks[i];
        if (!stk.isNull()) {
            auto kdata = stk.getKData(KQueryByDate(yesterday, tomorrow));
            if (kdata.size() == 2) {
                HKU_ASSERT(code_builder.Append(stk.market_code()).ok());
                HKU_ASSERT(name_builder.Append(stk.name()).ok());
                HKU_ASSERT(date_builder.Append(kdata[1].datetime.timestamp() / 1000LL).ok());
                HKU_ASSERT(open_builder.Append(kdata[1].openPrice).ok());
                HKU_ASSERT(high_builder.Append(kdata[1].highPrice).ok());
                HKU_ASSERT(low_builder.Append(kdata[1].lowPrice).ok());
                HKU_ASSERT(close_builder.Append(kdata[1].closePrice).ok());
                HKU_ASSERT(amount_builder.Append(kdata[1].transAmount).ok());
                HKU_ASSERT(volume_builder.Append(kdata[1].transCount).ok());
                HKU_ASSERT(yesterday_close_builder.Append(kdata[0].closePrice).ok());
                Indicator::value_t hsl = HSL(kdata)[1];
                HKU_ASSERT(turnover_builder.Append(hsl * 100.0).ok());
                HKU_ASSERT(
                  amplitude_builder
                    .Append(kdata[1].openPrice != 0.0
                              ? ((kdata[1].highPrice - kdata[1].lowPrice) / kdata[1].openPrice) *
                                  100.0
                              : Null<price_t>())
                    .ok());
                HKU_ASSERT(price_change_builder
                             .Append(kdata[0].closePrice != 0.0
                                       ? ((kdata[1].closePrice - kdata[0].closePrice) /
                                          kdata[0].closePrice) *
                                           100.0
                                       : Null<price_t>())
                             .ok());
                Indicator::value_t zongguben = ZONGGUBEN(kdata)[1];
                HKU_ASSERT(total_market_cap_builder.Append(zongguben * kdata[1].closePrice).ok());
                Indicator::value_t liutongpan = LIUTONGPAN(kdata)[1];
                HKU_ASSERT(
                  circulating_market_cap_builder.Append(liutongpan * kdata[1].closePrice).ok());
                Indicator::value_t pe = PE(kdata)[1];
                HKU_ASSERT(pb_builder.Append(pe).ok());
                Indicator::value_t pb = PB(kdata)[1];
                HKU_ASSERT(pe_builder.Append(pb).ok());

            } else if (kdata.size() == 1 && kdata[0].datetime == dates[1]) {
                HKU_ASSERT(code_builder.Append(stk.market_code()).ok());
                HKU_ASSERT(name_builder.Append(stk.name()).ok());
                HKU_ASSERT(date_builder.Append(dates[1].timestamp() / 1000LL).ok());
                HKU_ASSERT(open_builder.Append(kdata[0].openPrice).ok());
                HKU_ASSERT(high_builder.Append(kdata[0].highPrice).ok());
                HKU_ASSERT(low_builder.Append(kdata[0].lowPrice).ok());
                HKU_ASSERT(close_builder.Append(kdata[0].closePrice).ok());
                HKU_ASSERT(amount_builder.Append(kdata[0].transAmount).ok());
                HKU_ASSERT(volume_builder.Append(kdata[0].transCount).ok());
                Indicator::value_t hsl = HSL(kdata)[0];
                HKU_ASSERT(turnover_builder.Append(hsl * 100.).ok());
                HKU_ASSERT(
                  amplitude_builder
                    .Append(kdata[0].openPrice != 0.0
                              ? ((kdata[0].highPrice - kdata[0].lowPrice) / kdata[0].openPrice) *
                                  100.0
                              : Null<price_t>())
                    .ok());
                HKU_ASSERT(
                  price_change_builder
                    .Append(kdata[0].openPrice != 0.0
                              ? ((kdata[0].closePrice - kdata[0].openPrice) / kdata[0].openPrice) *
                                  100.0
                              : Null<price_t>())
                    .ok());
                Indicator::value_t zongguben = ZONGGUBEN(kdata)[0];
                HKU_ASSERT(total_market_cap_builder.Append(zongguben * kdata[0].closePrice).ok());
                Indicator::value_t liutongpan = LIUTONGPAN(kdata)[0];
                HKU_ASSERT(
                  circulating_market_cap_builder.Append(liutongpan * kdata[0].closePrice).ok());
                Indicator::value_t pe = PE(kdata)[0];
                HKU_ASSERT(pb_builder.Append(pe).ok());
                Indicator::value_t pb = PB(kdata)[0];
                HKU_ASSERT(pe_builder.Append(pb).ok());
            }
        }
    }

    std::shared_ptr<arrow::Array> code_arr, name_arr, date_arr, open_arr, high_arr, low_arr,
      close_arr, amount_arr, volume_arr, yesterday_close_arr, turnover_arr, amplitude_arr,
      price_change_arr, total_market_cap_arr, circulating_market_cap_arr, pe_arr, pb_arr;

    HKU_ERROR_IF_RETURN(!code_builder.Finish(&code_arr).ok(), ret, "Failed to build code array!");
    HKU_ERROR_IF_RETURN(!name_builder.Finish(&name_arr).ok(), ret, "Failed to build name array!");
    HKU_ERROR_IF_RETURN(!date_builder.Finish(&date_arr).ok(), ret, "Failed to build date array!");
    HKU_ERROR_IF_RETURN(!open_builder.Finish(&open_arr).ok(), ret, "Failed to build open array!");
    HKU_ERROR_IF_RETURN(!high_builder.Finish(&high_arr).ok(), ret, "Failed to build high array!");
    HKU_ERROR_IF_RETURN(!low_builder.Finish(&low_arr).ok(), ret, "Failed to build low array!");
    HKU_ERROR_IF_RETURN(!close_builder.Finish(&close_arr).ok(), ret,
                        "Failed to build close array!");
    HKU_ERROR_IF_RETURN(!amount_builder.Finish(&amount_arr).ok(), ret,
                        "Failed to build amount array!");
    HKU_ERROR_IF_RETURN(!volume_builder.Finish(&volume_arr).ok(), ret,
                        "Failed to build volume array!");
    HKU_ERROR_IF_RETURN(!yesterday_close_builder.Finish(&yesterday_close_arr).ok(), ret,
                        "Failed to build yesterday_close array!");
    HKU_ERROR_IF_RETURN(!turnover_builder.Finish(&turnover_arr).ok(), ret,
                        "Failed to build turnover array!");
    HKU_ERROR_IF_RETURN(!amplitude_builder.Finish(&amplitude_arr).ok(), ret,
                        "Failed to build amplitude array!");
    HKU_ERROR_IF_RETURN(!price_change_builder.Finish(&price_change_arr).ok(), ret,
                        "Failed to build price_change array!");
    HKU_ERROR_IF_RETURN(!total_market_cap_builder.Finish(&total_market_cap_arr).ok(), ret,
                        "Failed to build total_market_cap array!");
    HKU_ERROR_IF_RETURN(!circulating_market_cap_builder.Finish(&circulating_market_cap_arr).ok(),
                        ret, "Failed to build circulating_market_cap array!");
    HKU_ERROR_IF_RETURN(!pe_builder.Finish(&pe_arr).ok(), ret, "Failed to build pe array!");
    HKU_ERROR_IF_RETURN(!pb_builder.Finish(&pb_arr).ok(), ret, "Failed to build pb array!");

    // 创建Schema
    auto schema = arrow::schema({
      arrow::field(htr("market_code"), arrow::utf8()),
      arrow::field(htr("stock_name"), arrow::utf8()),
      arrow::field(htr("date"), arrow::date64()),
      arrow::field(htr("open"), arrow::float64()),
      arrow::field(htr("high"), arrow::float64()),
      arrow::field(htr("low"), arrow::float64()),
      arrow::field(htr("close"), arrow::float64()),
      arrow::field(htr("amount"), arrow::float64()),
      arrow::field(htr("volume"), arrow::float64()),
      arrow::field(htr("yesterday_close"), arrow::float64()),
      arrow::field(htr("turnover"), arrow::float64()),
      arrow::field(htr("amplitude"), arrow::float64()),
      arrow::field(htr("price_change"), arrow::float64()),
      arrow::field(htr("total_market_cap"), arrow::float64()),
      arrow::field(htr("circulating_market_cap"), arrow::float64()),
      arrow::field(htr("pe"), arrow::float64()),
      arrow::field(htr("pb"), arrow::float64()),
    });

    ret = arrow::Table::Make(schema, {
                                       code_arr,
                                       name_arr,
                                       date_arr,
                                       open_arr,
                                       high_arr,
                                       low_arr,
                                       close_arr,
                                       amount_arr,
                                       volume_arr,
                                       yesterday_close_arr,
                                       turnover_arr,
                                       amplitude_arr,
                                       price_change_arr,
                                       total_market_cap_arr,
                                       circulating_market_cap_arr,
                                       pe_arr,
                                       pb_arr,
                                     });

    HKU_ASSERT(ret);

    return ret;
}

std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks, const string& market) {
    std::shared_ptr<arrow::Table> ret;
    DatetimeList dates = StockManager::instance().getTradingCalendar(KQueryByIndex(-1), market);
    HKU_IF_RETURN(dates.empty(), ret);

    ret = getMarketView(stks, dates.back(), market);
    return ret;
}

std::shared_ptr<arrow::Table> HKU_API getIndicatorsView(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const Datetime& date,
                                                        const string& market) {
    std::shared_ptr<arrow::Table> ret;
    auto& sm = StockManager::instance();
    MarketInfo info = sm.getMarketInfo(market);
    HKU_IF_RETURN(info.code().empty(), ret);

    Stock market_stk = sm.getStock(fmt::format("{}{}", market, info.code()));
    HKU_IF_RETURN(market_stk.isNull(), ret);

    return ret;
}

}  // namespace hku