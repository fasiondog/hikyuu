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
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/StockManager.h"
#include "MarketView.h"

namespace hku {

arrow::Result<std::shared_ptr<arrow::Table>> HKU_API getMarketView(const StockList& stks,
                                                                   const Datetime& date,
                                                                   const string& market) {
    arrow::Result<std::shared_ptr<arrow::Table>> ret;
    auto& sm = StockManager::instance();
    MarketInfo info = sm.getMarketInfo(market);
    HKU_ARROW_ERROR_IF_RETURN(info.code().empty(), "Can not find market info for {}!", market);

    Stock market_stk = sm.getStock(fmt::format("{}{}", market, info.code()));
    HKU_ARROW_ERROR_IF_RETURN(market_stk.isNull(), "Can not find market stock for {}!", market);

    size_t start_pos, end_pos;
    HKU_ARROW_ERROR_IF_RETURN(
      !market_stk.getIndexRange(KQueryByDate(date.startOfDay(), date.nextDay()), start_pos,
                                end_pos),
      "Unknown error!");

    HKU_ARROW_ERROR_IF_RETURN(start_pos < 1, "At least 2 trading days are required!");

    DatetimeList dates = market_stk.getDatetimeList(KQueryByIndex(start_pos - 1, start_pos + 1));
    HKU_ARROW_ERROR_IF_RETURN(dates.size() < 2, "At least 2 trading days are required!");

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
    HKU_ARROW_RETURN_NOT_OK(code_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(open_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(high_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(low_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(close_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(amount_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(volume_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(yesterday_close_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(turnover_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(amplitude_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(price_change_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(total_market_cap_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(circulating_market_cap_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(pe_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(pb_builder.Reserve(total));

    for (size_t i = 0; i < total; i++) {
        const Stock& stk = stks[i];
        if (!stk.isNull()) {
            auto kdata = stk.getKData(KQueryByDate(yesterday, tomorrow));
            if (kdata.size() == 2) {
                HKU_ARROW_RETURN_NOT_OK(code_builder.Append(stk.market_code()));
                HKU_ARROW_RETURN_NOT_OK(name_builder.Append(stk.name()));
                HKU_ARROW_RETURN_NOT_OK(
                  date_builder.Append(kdata[1].datetime.timestamp() / 1000LL));
                HKU_ARROW_RETURN_NOT_OK(open_builder.Append(kdata[1].openPrice));
                HKU_ARROW_RETURN_NOT_OK(high_builder.Append(kdata[1].highPrice));
                HKU_ARROW_RETURN_NOT_OK(low_builder.Append(kdata[1].lowPrice));
                HKU_ARROW_RETURN_NOT_OK(close_builder.Append(kdata[1].closePrice));
                HKU_ARROW_RETURN_NOT_OK(amount_builder.Append(kdata[1].transAmount));
                HKU_ARROW_RETURN_NOT_OK(volume_builder.Append(kdata[1].transCount));
                HKU_ARROW_RETURN_NOT_OK(yesterday_close_builder.Append(kdata[0].closePrice));
                Indicator::value_t hsl = HSL(kdata)[1];
                HKU_ARROW_RETURN_NOT_OK(turnover_builder.Append(hsl * 100.0));
                HKU_ARROW_RETURN_NOT_OK(amplitude_builder.Append(
                  kdata[1].openPrice != 0.0
                    ? ((kdata[1].highPrice - kdata[1].lowPrice) / kdata[1].openPrice) * 100.0
                    : Null<price_t>()));
                HKU_ARROW_RETURN_NOT_OK(price_change_builder.Append(
                  kdata[0].closePrice != 0.0
                    ? ((kdata[1].closePrice - kdata[0].closePrice) / kdata[0].closePrice) * 100.0
                    : Null<price_t>()));
                Indicator::value_t zongguben = ZONGGUBEN(kdata)[1];
                HKU_ARROW_RETURN_NOT_OK(
                  total_market_cap_builder.Append(zongguben * kdata[1].closePrice));
                Indicator::value_t liutongpan = LIUTONGPAN(kdata)[1];
                HKU_ARROW_RETURN_NOT_OK(
                  circulating_market_cap_builder.Append(liutongpan * kdata[1].closePrice));
                Indicator::value_t pe = PE(kdata)[1];
                HKU_ARROW_RETURN_NOT_OK(pb_builder.Append(pe));
                Indicator::value_t pb = PB(kdata)[1];
                HKU_ARROW_RETURN_NOT_OK(pe_builder.Append(pb));

            } else if (kdata.size() == 1 && kdata[0].datetime == dates[1]) {
                HKU_ARROW_RETURN_NOT_OK(code_builder.Append(stk.market_code()));
                HKU_ARROW_RETURN_NOT_OK(name_builder.Append(stk.name()));
                HKU_ARROW_RETURN_NOT_OK(date_builder.Append(dates[1].timestamp() / 1000LL));
                HKU_ARROW_RETURN_NOT_OK(open_builder.Append(kdata[0].openPrice));
                HKU_ARROW_RETURN_NOT_OK(high_builder.Append(kdata[0].highPrice));
                HKU_ARROW_RETURN_NOT_OK(low_builder.Append(kdata[0].lowPrice));
                HKU_ARROW_RETURN_NOT_OK(close_builder.Append(kdata[0].closePrice));
                HKU_ARROW_RETURN_NOT_OK(amount_builder.Append(kdata[0].transAmount));
                HKU_ARROW_RETURN_NOT_OK(volume_builder.Append(kdata[0].transCount));
                Indicator::value_t hsl = HSL(kdata)[0];
                HKU_ARROW_RETURN_NOT_OK(turnover_builder.Append(hsl * 100.));
                HKU_ARROW_RETURN_NOT_OK(amplitude_builder.Append(
                  kdata[0].openPrice != 0.0
                    ? ((kdata[0].highPrice - kdata[0].lowPrice) / kdata[0].openPrice) * 100.0
                    : Null<price_t>()));
                HKU_ARROW_RETURN_NOT_OK(price_change_builder.Append(
                  kdata[0].openPrice != 0.0
                    ? ((kdata[0].closePrice - kdata[0].openPrice) / kdata[0].openPrice) * 100.0
                    : Null<price_t>()));
                Indicator::value_t zongguben = ZONGGUBEN(kdata)[0];
                HKU_ARROW_RETURN_NOT_OK(
                  total_market_cap_builder.Append(zongguben * kdata[0].closePrice));
                Indicator::value_t liutongpan = LIUTONGPAN(kdata)[0];
                HKU_ARROW_RETURN_NOT_OK(
                  circulating_market_cap_builder.Append(liutongpan * kdata[0].closePrice));
                Indicator::value_t pe = PE(kdata)[0];
                HKU_ARROW_RETURN_NOT_OK(pb_builder.Append(pe));
                Indicator::value_t pb = PB(kdata)[0];
                HKU_ARROW_RETURN_NOT_OK(pe_builder.Append(pb));
            }
        }
    }

    std::shared_ptr<arrow::Array> code_arr, name_arr, date_arr, open_arr, high_arr, low_arr,
      close_arr, amount_arr, volume_arr, yesterday_close_arr, turnover_arr, amplitude_arr,
      price_change_arr, total_market_cap_arr, circulating_market_cap_arr, pe_arr, pb_arr;

    HKU_ARROW_RETURN_NOT_OK(code_builder.Finish(&code_arr));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Finish(&name_arr));
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(open_builder.Finish(&open_arr));
    HKU_ARROW_RETURN_NOT_OK(high_builder.Finish(&high_arr));
    HKU_ARROW_RETURN_NOT_OK(low_builder.Finish(&low_arr));
    HKU_ARROW_RETURN_NOT_OK(close_builder.Finish(&close_arr));
    HKU_ARROW_RETURN_NOT_OK(amount_builder.Finish(&amount_arr));
    HKU_ARROW_RETURN_NOT_OK(volume_builder.Finish(&volume_arr));
    HKU_ARROW_RETURN_NOT_OK(yesterday_close_builder.Finish(&yesterday_close_arr));
    HKU_ARROW_RETURN_NOT_OK(turnover_builder.Finish(&turnover_arr));
    HKU_ARROW_RETURN_NOT_OK(amplitude_builder.Finish(&amplitude_arr));
    HKU_ARROW_RETURN_NOT_OK(price_change_builder.Finish(&price_change_arr));
    HKU_ARROW_RETURN_NOT_OK(total_market_cap_builder.Finish(&total_market_cap_arr));
    HKU_ARROW_RETURN_NOT_OK(circulating_market_cap_builder.Finish(&circulating_market_cap_arr));
    HKU_ARROW_RETURN_NOT_OK(pe_builder.Finish(&pe_arr));
    HKU_ARROW_RETURN_NOT_OK(pb_builder.Finish(&pb_arr));

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

    return arrow::Table::Make(schema, {
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
}

arrow::Result<std::shared_ptr<arrow::Table>> HKU_API getMarketView(const StockList& stks,
                                                                   const string& market) {
    arrow::Result<std::shared_ptr<arrow::Table>> ret;
    DatetimeList dates = StockManager::instance().getTradingCalendar(KQueryByIndex(-1), market);
    HKU_ARROW_IF_RETURN(dates.empty());

    return getMarketView(stks, dates.back(), market);
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API getIndicatorsView(
  const StockList& stks, const IndicatorList& inds, const KQuery& query, const string& market) {
    auto& sm = StockManager::instance();
    auto dates = sm.getTradingCalendar(query, market);

    std::vector<std::shared_ptr<arrow::Field>> fields;
    fields.reserve(inds.size() + 3);
    fields.emplace_back(arrow::field(htr("market_code"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("stock_name"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("date"), arrow::date64()));
    for (auto& ind : inds) {
        fields.emplace_back(arrow::field(ind.name(), arrow::float64()));
    }

    arrow::StringBuilder code_builder, name_builder;
    arrow::Date64Builder date_builder;
    std::vector<arrow::DoubleBuilder> builders(inds.size());
    for (const auto& stk : stks) {
        auto kdata = stk.getKData(query);
        bool have_data = true;
        for (size_t i = 0; i < inds.size(); ++i) {
            auto x = ALIGN(inds[i], dates, true)(kdata);
            if (x.size() != dates.size()) {
                have_data = false;
            } else {
                const auto* x_ptr = x.data();
                HKU_ARROW_RETURN_NOT_OK(builders[i].AppendValues(x_ptr, x.size()));
            }
        }

        if (have_data) {
            for (size_t i = 0; i < dates.size(); ++i) {
                HKU_ARROW_RETURN_NOT_OK(code_builder.Append(stk.market_code()));
                HKU_ARROW_RETURN_NOT_OK(name_builder.Append(stk.name()));
                HKU_ARROW_RETURN_NOT_OK(date_builder.Append(dates[i].timestamp() / 1000LL));
            }
        }
    }

    vector<std::shared_ptr<arrow::Array>> arrs;
    arrs.reserve(inds.size() + 3);

    auto code_arr = code_builder.Finish();
    HKU_ARROW_RETURN_NOT_OK2(code_arr);
    arrs.push_back(*code_arr);

    auto name_arr = name_builder.Finish();
    HKU_ARROW_RETURN_NOT_OK2(name_arr);
    arrs.push_back(*name_arr);

    auto date_arr = date_builder.Finish();
    HKU_ARROW_RETURN_NOT_OK2(date_arr);
    arrs.push_back(*date_arr);

    for (size_t i = 0; i < inds.size(); ++i) {
        auto arr = builders[i].Finish();
        HKU_ARROW_RETURN_NOT_OK2(arr);
        arrs.push_back(*arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getIndicatorsView(const StockList& stks, const IndicatorList& inds, const Datetime& date,
                  size_t cal_len, const KQuery::KType& ktype, const string& market) {
    auto& sm = StockManager::instance();
    MarketInfo info = sm.getMarketInfo(market);
    HKU_ARROW_ERROR_IF_RETURN(info.code().empty(), "Can not find market info for {}!", market);

    Stock market_stk = sm.getStock(fmt::format("{}{}", market, info.code()));
    HKU_ARROW_ERROR_IF_RETURN(market_stk.isNull(), "Can not find market stock for {}!", market);

    std::vector<std::shared_ptr<arrow::Field>> fields;
    fields.reserve(inds.size() + 3);
    fields.emplace_back(arrow::field(htr("market_code"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("stock_name"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("date"), arrow::date64()));
    for (auto& ind : inds) {
        fields.emplace_back(arrow::field(ind.name(), HKU_ARROW_PRICE_FIELD));
    }

    arrow::StringBuilder code_builder, name_builder;
    arrow::Date64Builder date_builder;
    std::vector<HKU_ARROW_PRICE_BUILDER> builders(inds.size());

    size_t start_pos, end_pos;

    bool success = market_stk.getIndexRange(
      KQueryByDate(date, date + Minutes(KQuery::getKTypeInMin(ktype)), ktype), start_pos, end_pos);

    if (success) {
        if (start_pos > cal_len) {
            start_pos -= cal_len;
        }

        DatetimeList dates = market_stk.getDatetimeList(KQueryByIndex(start_pos, end_pos, ktype));
        if (!dates.empty() && date == dates.back()) {
            KQuery query =
              KQueryByDate(dates.front(), date + Minutes(KQuery::getKTypeInMin(ktype)), ktype);

            for (const auto& stk : stks) {
                auto kdata = stk.getKData(query);
                bool have_data = true;
                for (size_t i = 0; i < inds.size(); ++i) {
                    auto x = ALIGN(inds[i], dates, true)(kdata);
                    if (x.size() != dates.size()) {
                        have_data = false;
                    } else {
                        HKU_ARROW_RETURN_NOT_OK(builders[i].Append(x[dates.size() - 1]));
                    }
                }

                if (have_data) {
                    HKU_ARROW_RETURN_NOT_OK(code_builder.Append(stk.market_code()));
                    HKU_ARROW_RETURN_NOT_OK(name_builder.Append(stk.name()));
                    HKU_ARROW_RETURN_NOT_OK(date_builder.Append(dates.back().timestamp() / 1000LL));
                }
            }
        }
    }

    vector<std::shared_ptr<arrow::Array>> arrs;
    arrs.reserve(inds.size() + 3);

    auto code_arr = code_builder.Finish();
    HKU_ARROW_RETURN_NOT_OK2(code_arr);
    arrs.push_back(*code_arr);

    auto name_arr = name_builder.Finish();
    HKU_ARROW_RETURN_NOT_OK2(name_arr);
    arrs.push_back(*name_arr);

    auto date_arr = date_builder.Finish();
    HKU_ARROW_RETURN_NOT_OK2(date_arr);
    arrs.push_back(*date_arr);

    for (size_t i = 0; i < inds.size(); ++i) {
        auto arr = builders[i].Finish();
        HKU_ARROW_RETURN_NOT_OK2(arr);
        arrs.push_back(*arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

}  // namespace hku