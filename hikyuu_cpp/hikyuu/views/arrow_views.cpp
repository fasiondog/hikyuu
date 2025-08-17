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
#include "arrow_views.h"

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
    fields.emplace_back(arrow::field(htr("datetime"), arrow::date64()));
    for (auto& ind : inds) {
        fields.emplace_back(arrow::field(ind.name(), arrow::float64()));
    }

    arrow::StringBuilder code_builder, name_builder;
    arrow::Date64Builder date_builder;
    std::vector<arrow::DoubleBuilder> builders(inds.size());

    size_t total = stks.size() * dates.size();
    HKU_ARROW_RETURN_NOT_OK(code_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    for (auto& builder : builders) {
        HKU_ARROW_RETURN_NOT_OK(builder.Reserve(total));
    }

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
    fields.emplace_back(arrow::field(htr("datetime"), arrow::date64()));
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

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getKRecordListView(const KRecordList& ks) {
    arrow::Date64Builder date_builder;
    arrow::DoubleBuilder open_builder, high_builder, low_builder, close_builder, amount_builder,
      volume_builder;
    size_t total = ks.size();
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(open_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(high_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(low_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(close_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(amount_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(volume_builder.Reserve(total));

    for (auto& k : ks) {
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(k.datetime.timestamp() / 1000LL));
        HKU_ARROW_RETURN_NOT_OK(open_builder.Append(k.openPrice));
        HKU_ARROW_RETURN_NOT_OK(high_builder.Append(k.highPrice));
        HKU_ARROW_RETURN_NOT_OK(low_builder.Append(k.lowPrice));
        HKU_ARROW_RETURN_NOT_OK(close_builder.Append(k.closePrice));
        HKU_ARROW_RETURN_NOT_OK(amount_builder.Append(k.transAmount));
        HKU_ARROW_RETURN_NOT_OK(volume_builder.Append(k.transCount));
    }

    std::shared_ptr<arrow::Array> date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr,
      volume_arr;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(open_builder.Finish(&open_arr));
    HKU_ARROW_RETURN_NOT_OK(high_builder.Finish(&high_arr));
    HKU_ARROW_RETURN_NOT_OK(low_builder.Finish(&low_arr));
    HKU_ARROW_RETURN_NOT_OK(close_builder.Finish(&close_arr));
    HKU_ARROW_RETURN_NOT_OK(amount_builder.Finish(&amount_arr));
    HKU_ARROW_RETURN_NOT_OK(volume_builder.Finish(&volume_arr));

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::date64()),
      arrow::field("open", arrow::float64()),
      arrow::field("high", arrow::float64()),
      arrow::field("low", arrow::float64()),
      arrow::field("close", arrow::float64()),
      arrow::field("amount", arrow::float64()),
      arrow::field("volume", arrow::float64()),
    });

    return arrow::Table::Make(
      schema, {date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr, volume_arr});
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getTimeLineListView(const TimeLineList& ts) {
    arrow::Date64Builder date_builder;
    arrow::DoubleBuilder price_builder, vol_builder;
    size_t total = ts.size();
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(price_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(vol_builder.Reserve(total));

    for (auto& t : ts) {
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(t.datetime.timestamp() / 1000LL));
        HKU_ARROW_RETURN_NOT_OK(price_builder.Append(t.price));
        HKU_ARROW_RETURN_NOT_OK(vol_builder.Append(t.vol));
    }

    std::shared_ptr<arrow::Array> date_arr, price_arr, vol_arr;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(price_builder.Finish(&price_arr));
    HKU_ARROW_RETURN_NOT_OK(vol_builder.Finish(&vol_arr));

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::date64()),
      arrow::field("price", arrow::float64()),
      arrow::field("vol", arrow::float64()),
    });

    return arrow::Table::Make(schema, {date_arr, price_arr, vol_arr});
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getTransRecordListView(const TransRecordList& ts) {
    arrow::Date64Builder date_builder;
    arrow::DoubleBuilder price_builder, vol_builder;
    arrow::Int32Builder direct_builder;
    size_t total = ts.size();
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(price_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(vol_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(direct_builder.Reserve(total));

    for (auto& t : ts) {
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(t.datetime.timestamp() / 1000LL));
        HKU_ARROW_RETURN_NOT_OK(price_builder.Append(t.price));
        HKU_ARROW_RETURN_NOT_OK(vol_builder.Append(t.vol));
        HKU_ARROW_RETURN_NOT_OK(direct_builder.Append(t.direct));
    }

    std::shared_ptr<arrow::Array> date_arr, price_arr, vol_arr, direct_arr;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(price_builder.Finish(&price_arr));
    HKU_ARROW_RETURN_NOT_OK(vol_builder.Finish(&vol_arr));
    HKU_ARROW_RETURN_NOT_OK(direct_builder.Finish(&direct_arr));

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::date64()),
      arrow::field("price", arrow::float64()),
      arrow::field("vol", arrow::float64()),
      arrow::field("direct", arrow::int32()),
    });

    return arrow::Table::Make(schema, {date_arr, price_arr, vol_arr, direct_arr});
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getStockWeightListView(const StockWeightList& sws) {
    arrow::Date64Builder date_builder;
    arrow::DoubleBuilder countAsGift_builder, countForSell_builder, priceForSell_builder,
      bonus_builder, increasement_builder, totalCount_builder, freeCount_builder, suogu_builder;
    size_t total = sws.size();
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(countAsGift_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(countForSell_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(priceForSell_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(bonus_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(increasement_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(totalCount_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(freeCount_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(suogu_builder.Reserve(total));

    for (auto& w : sws) {
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(w.datetime().timestamp() / 1000LL));
        HKU_ARROW_RETURN_NOT_OK(countAsGift_builder.Append(w.countAsGift()));
        HKU_ARROW_RETURN_NOT_OK(countForSell_builder.Append(w.countForSell()));
        HKU_ARROW_RETURN_NOT_OK(priceForSell_builder.Append(w.priceForSell()));
        HKU_ARROW_RETURN_NOT_OK(bonus_builder.Append(w.bonus()));
        HKU_ARROW_RETURN_NOT_OK(increasement_builder.Append(w.increasement()));
        HKU_ARROW_RETURN_NOT_OK(totalCount_builder.Append(w.totalCount()));
        HKU_ARROW_RETURN_NOT_OK(freeCount_builder.Append(w.freeCount()));
        HKU_ARROW_RETURN_NOT_OK(suogu_builder.Append(w.suogu()));
    }

    std::shared_ptr<arrow::Array> date_arr, countAsGift_arr, countForSell_arr, priceForSell_arr,
      bonus_arr, increasement_arr, totalCount_arr, freeCount_arr, suogu_arr;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(countAsGift_builder.Finish(&countAsGift_arr));
    HKU_ARROW_RETURN_NOT_OK(countForSell_builder.Finish(&countForSell_arr));
    HKU_ARROW_RETURN_NOT_OK(priceForSell_builder.Finish(&priceForSell_arr));
    HKU_ARROW_RETURN_NOT_OK(bonus_builder.Finish(&bonus_arr));
    HKU_ARROW_RETURN_NOT_OK(increasement_builder.Finish(&increasement_arr));
    HKU_ARROW_RETURN_NOT_OK(totalCount_builder.Finish(&totalCount_arr));
    HKU_ARROW_RETURN_NOT_OK(freeCount_builder.Finish(&freeCount_arr));
    HKU_ARROW_RETURN_NOT_OK(suogu_builder.Finish(&suogu_arr));

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::date64()),
      arrow::field("countAsGift", arrow::float64()),
      arrow::field("countForSell", arrow::float64()),
      arrow::field("priceForSell", arrow::float64()),
      arrow::field("bonus", arrow::float64()),
      arrow::field("increasement", arrow::float64()),
      arrow::field("totalCount", arrow::float64()),
      arrow::field("freeCount", arrow::float64()),
      arrow::field("suogu", arrow::float64()),
    });

    return arrow::Table::Make(
      schema, {date_arr, countAsGift_arr, countForSell_arr, priceForSell_arr, bonus_arr,
               increasement_arr, totalCount_arr, freeCount_arr, suogu_arr});
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getDatetimeListView(const DatetimeList& dates) {
    arrow::Date64Builder date_builder;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(dates.size()));

    for (auto& date : dates) {
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(date.timestamp() / 1000LL));
    }

    std::shared_ptr<arrow::Array> date_arr;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));

    auto schema = arrow::schema({arrow::field("datetime", arrow::date64())});
    return arrow::Table::Make(schema, {date_arr});
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getTradeRecordListView(const TradeRecordList& trades) {
    arrow::StringBuilder code_builder, name_builder, business_builder, sig_builder, remark_builder;
    arrow::Date64Builder date_builder;
    arrow::DoubleBuilder plan_builder, real_builder, goal_builder, stoploss_builder, number_builder,
      cash_builder, cost_total_builder, cost_commission_builder, cost_stamptax_builder,
      cost_transferfee_builder, cost_other_builder;

    size_t total = trades.size();
    HKU_ARROW_RETURN_NOT_OK(code_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(business_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(plan_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(real_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(goal_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(number_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(stoploss_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cash_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cost_total_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cost_commission_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cost_stamptax_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cost_transferfee_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cost_other_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(sig_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(remark_builder.Reserve(total));

    for (auto& tr : trades) {
        HKU_ARROW_RETURN_NOT_OK(code_builder.Append(tr.stock.market_code()));
        HKU_ARROW_RETURN_NOT_OK(name_builder.Append(tr.stock.name()));
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(tr.datetime.timestamp() / 1000LL));
        HKU_ARROW_RETURN_NOT_OK(business_builder.Append(getBusinessName(tr.business)));
        HKU_ARROW_RETURN_NOT_OK(plan_builder.Append(tr.planPrice));
        HKU_ARROW_RETURN_NOT_OK(real_builder.Append(tr.realPrice));
        HKU_ARROW_RETURN_NOT_OK(goal_builder.Append(tr.goalPrice));
        HKU_ARROW_RETURN_NOT_OK(number_builder.Append(tr.number));
        HKU_ARROW_RETURN_NOT_OK(stoploss_builder.Append(tr.stoploss));
        HKU_ARROW_RETURN_NOT_OK(cash_builder.Append(tr.cash));
        HKU_ARROW_RETURN_NOT_OK(cost_total_builder.Append(tr.cost.total));
        HKU_ARROW_RETURN_NOT_OK(cost_commission_builder.Append(tr.cost.commission));
        HKU_ARROW_RETURN_NOT_OK(cost_stamptax_builder.Append(tr.cost.stamptax));
        HKU_ARROW_RETURN_NOT_OK(cost_transferfee_builder.Append(tr.cost.transferfee));
        HKU_ARROW_RETURN_NOT_OK(cost_other_builder.Append(tr.cost.others));
        HKU_ARROW_RETURN_NOT_OK(sig_builder.Append(getSystemPartName(tr.from)));
        HKU_ARROW_RETURN_NOT_OK(remark_builder.Append(tr.remark));
    }

    std::shared_ptr<arrow::Array> code_arr, name_arr, date_arr, business_arr, plan_arr, real_arr,
      goal_arr, number_arr, stoploss_arr, cash_arr, cost_total_arr, cost_commission_arr,
      cost_stamptax_arr, cost_transferfee_arr, cost_other_arr, sig_arr, remark_arr;

    HKU_ARROW_RETURN_NOT_OK(code_builder.Finish(&code_arr));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Finish(&name_arr));
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(business_builder.Finish(&business_arr));
    HKU_ARROW_RETURN_NOT_OK(plan_builder.Finish(&plan_arr));
    HKU_ARROW_RETURN_NOT_OK(real_builder.Finish(&real_arr));
    HKU_ARROW_RETURN_NOT_OK(goal_builder.Finish(&goal_arr));
    HKU_ARROW_RETURN_NOT_OK(number_builder.Finish(&number_arr));
    HKU_ARROW_RETURN_NOT_OK(stoploss_builder.Finish(&stoploss_arr));
    HKU_ARROW_RETURN_NOT_OK(cash_builder.Finish(&cash_arr));
    HKU_ARROW_RETURN_NOT_OK(cost_total_builder.Finish(&cost_total_arr));
    HKU_ARROW_RETURN_NOT_OK(cost_commission_builder.Finish(&cost_commission_arr));
    HKU_ARROW_RETURN_NOT_OK(cost_stamptax_builder.Finish(&cost_stamptax_arr));
    HKU_ARROW_RETURN_NOT_OK(cost_transferfee_builder.Finish(&cost_transferfee_arr));
    HKU_ARROW_RETURN_NOT_OK(cost_other_builder.Finish(&cost_other_arr));
    HKU_ARROW_RETURN_NOT_OK(sig_builder.Finish(&sig_arr));
    HKU_ARROW_RETURN_NOT_OK(remark_builder.Finish(&remark_arr));

    auto schema = arrow::schema({
      arrow::field(htr("market_code"), arrow::utf8()),
      arrow::field(htr("stock_name"), arrow::utf8()),
      arrow::field(htr("datetime"), arrow::date64()),
      arrow::field(htr("business"), arrow::utf8()),
      arrow::field(htr("planPrice"), arrow::float64()),
      arrow::field(htr("realPrice"), arrow::float64()),
      arrow::field(htr("goalPrice"), arrow::float64()),
      arrow::field(htr("number"), arrow::float64()),
      arrow::field(htr("stoploss"), arrow::float64()),
      arrow::field(htr("cash"), arrow::float64()),
      arrow::field(htr("cost_total"), arrow::float64()),
      arrow::field(htr("cost_commission"), arrow::float64()),
      arrow::field(htr("cost_stamptax"), arrow::float64()),
      arrow::field(htr("cost_transferfee"), arrow::float64()),
      arrow::field(htr("cost_others"), arrow::float64()),
      arrow::field(htr("part_from"), arrow::utf8()),
      arrow::field(htr("remark"), arrow::utf8()),
    });

    return arrow::Table::Make(
      schema, {code_arr, name_arr, date_arr, business_arr, plan_arr, real_arr, goal_arr, number_arr,
               stoploss_arr, cash_arr, cost_total_arr, cost_commission_arr, cost_stamptax_arr,
               cost_transferfee_arr, cost_other_arr, sig_arr, remark_arr});
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getPositionRecordListView(const PositionRecordList& positions) {
    arrow::StringBuilder code_builder, name_builder;
    arrow::Date64Builder taketime_builder, cleantime_builder;
    arrow::Int64Builder hold_days_builder;
    arrow::DoubleBuilder hold_number_builder, invest_builder, market_value_builder, profit_builder,
      profit_percent_builder, stoploss_builder, goal_price_builder, total_number_builder,
      total_cost_builder, total_risk_builder, buy_money_builder, sell_money_builder;

    size_t total = positions.size();
    HKU_ARROW_RETURN_NOT_OK(code_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(taketime_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(cleantime_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(hold_days_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(hold_number_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(invest_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(market_value_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(profit_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(profit_percent_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(stoploss_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(goal_price_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(total_number_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(total_cost_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(total_risk_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(buy_money_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(sell_money_builder.Reserve(total));

    for (const auto& p : positions) {
        HKU_ARROW_RETURN_NOT_OK(code_builder.Append(p.stock.market_code()));
        HKU_ARROW_RETURN_NOT_OK(name_builder.Append(p.stock.name()));
        HKU_ARROW_RETURN_NOT_OK(taketime_builder.Append(p.takeDatetime.timestamp() / 1000LL));

        double invest = p.buyMoney - p.sellMoney + p.totalCost;
        double profit = 0.0;
        if (p.cleanDatetime.isNull()) {
            HKU_ARROW_RETURN_NOT_OK(cleantime_builder.AppendNull());
            double market_value = p.stock.getMarketValue(Datetime::now(), KQuery::DAY) * p.number;
            HKU_ARROW_RETURN_NOT_OK(
              hold_days_builder.Append((Datetime::now() - p.takeDatetime).days()));
            HKU_ARROW_RETURN_NOT_OK(market_value_builder.Append(market_value));
            profit = market_value - invest;
            HKU_ARROW_RETURN_NOT_OK(profit_builder.Append(profit));
        } else {
            HKU_ARROW_RETURN_NOT_OK(cleantime_builder.Append(p.cleanDatetime.timestamp() / 1000LL));
            HKU_ARROW_RETURN_NOT_OK(
              hold_days_builder.Append((p.cleanDatetime - p.takeDatetime).days()));
            HKU_ARROW_RETURN_NOT_OK(market_value_builder.Append(0.0));
            profit = p.totalProfit();
            HKU_ARROW_RETURN_NOT_OK(profit_builder.Append(profit));
        }
        HKU_ARROW_RETURN_NOT_OK(hold_number_builder.Append(p.number));
        HKU_ARROW_RETURN_NOT_OK(invest_builder.Append(invest));
        HKU_ARROW_RETURN_NOT_OK(profit_percent_builder.Append(
          invest != 0.0 ? roundEx(100. * (profit / invest), 2) : 0.0));
        HKU_ARROW_RETURN_NOT_OK(stoploss_builder.Append(p.stoploss));
        HKU_ARROW_RETURN_NOT_OK(goal_price_builder.Append(p.goalPrice));
        HKU_ARROW_RETURN_NOT_OK(total_number_builder.Append(p.totalNumber));
        HKU_ARROW_RETURN_NOT_OK(total_cost_builder.Append(p.totalCost));
        HKU_ARROW_RETURN_NOT_OK(total_risk_builder.Append(p.totalRisk));
        HKU_ARROW_RETURN_NOT_OK(buy_money_builder.Append(p.buyMoney));
        HKU_ARROW_RETURN_NOT_OK(sell_money_builder.Append(p.sellMoney));
    }

    std::shared_ptr<arrow::Array> code_arr, name_arr, taketime_arr, cleantime_arr, hold_days_arr,
      hold_number_arr, invest_arr, market_value_arr, profit_arr, profit_percent_arr, stoploss_arr,
      goal_price_arr, total_number_arr, total_cost_arr, total_risk_arr, buy_money_arr,
      sell_money_arr;

    HKU_ARROW_RETURN_NOT_OK(code_builder.Finish(&code_arr));
    HKU_ARROW_RETURN_NOT_OK(name_builder.Finish(&name_arr));
    HKU_ARROW_RETURN_NOT_OK(taketime_builder.Finish(&name_arr));
    HKU_ARROW_RETURN_NOT_OK(cleantime_builder.Finish(&cleantime_arr));
    HKU_ARROW_RETURN_NOT_OK(hold_days_builder.Finish(&hold_days_arr));
    HKU_ARROW_RETURN_NOT_OK(hold_number_builder.Finish(&hold_number_arr));
    HKU_ARROW_RETURN_NOT_OK(invest_builder.Finish(&invest_arr));
    HKU_ARROW_RETURN_NOT_OK(market_value_builder.Finish(&market_value_arr));
    HKU_ARROW_RETURN_NOT_OK(profit_builder.Finish(&profit_arr));
    HKU_ARROW_RETURN_NOT_OK(profit_percent_builder.Finish(&profit_percent_arr));
    HKU_ARROW_RETURN_NOT_OK(stoploss_builder.Finish(&stoploss_arr));
    HKU_ARROW_RETURN_NOT_OK(goal_price_builder.Finish(&goal_price_arr));
    HKU_ARROW_RETURN_NOT_OK(total_number_builder.Finish(&total_number_arr));
    HKU_ARROW_RETURN_NOT_OK(total_cost_builder.Finish(&total_cost_arr));
    HKU_ARROW_RETURN_NOT_OK(total_risk_builder.Finish(&total_risk_arr));
    HKU_ARROW_RETURN_NOT_OK(buy_money_builder.Finish(&buy_money_arr));
    HKU_ARROW_RETURN_NOT_OK(sell_money_builder.Finish(&sell_money_arr));

    auto schema = arrow::schema({
      arrow::field(htr("market_code"), arrow::utf8()),
      arrow::field(htr("stock_name"), arrow::utf8()),
      arrow::field(htr("take_time"), arrow::date64()),
      arrow::field(htr("hold_days"), arrow::int64()),
      arrow::field(htr("hold_number"), arrow::date64()),
      arrow::field(htr("invest"), arrow::float64()),
      arrow::field(htr("market_value"), arrow::float64()),
      arrow::field(htr("profit"), arrow::float64()),
      arrow::field(htr("profit_percent"), arrow::float64()),
      arrow::field(htr("stoploss"), arrow::float64()),
      arrow::field(htr("goal_price"), arrow::float64()),
      arrow::field(htr("clean_time"), arrow::date64()),
      arrow::field(htr("total_number"), arrow::float64()),
      arrow::field(htr("total_cost"), arrow::float64()),
      arrow::field(htr("total_risk"), arrow::float64()),
      arrow::field(htr("buy_money"), arrow::float64()),
      arrow::field(htr("sell_money"), arrow::float64()),
    });

    return arrow::Table::Make(
      schema, {code_arr, name_arr, taketime_arr, hold_days_arr, hold_number_arr, invest_arr,
               market_value_arr, profit_arr, profit_percent_arr, stoploss_arr, goal_price_arr,
               cleantime_arr, total_number_arr, total_cost_arr, total_risk_arr, buy_money_arr,
               sell_money_arr});
}

}  // namespace hku