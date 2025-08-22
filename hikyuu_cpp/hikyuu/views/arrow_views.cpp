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

static std::shared_ptr<arrow::Table> crtEmtpyMarketView() {
    // 创建Schema
    auto schema = arrow::schema({
      arrow::field(htr("market_code"), arrow::utf8()),
      arrow::field(htr("stock_name"), arrow::utf8()),
      arrow::field(htr("date"), arrow::timestamp(arrow::TimeUnit::NANO)),
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

    auto code_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(code_arr.ok());
    auto name_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(name_arr.ok());
    auto date_arr = arrow::MakeArrayFromScalar(
      arrow::TimestampScalar(arrow::timestamp(arrow::TimeUnit::NANO)), 0);
    HKU_ASSERT(date_arr.ok());
    auto open_arr = arrow::MakeArrayFromScalar(arrow::DoubleScalar(), 0);
    HKU_ASSERT(open_arr.ok());
    auto high_arr = arrow::MakeArrayFromScalar(arrow::DoubleScalar(), 0);
    HKU_ASSERT(high_arr.ok());
    auto low_arr = arrow::MakeArrayFromScalar(arrow::DoubleScalar(), 0);
    HKU_ASSERT(low_arr.ok());
    auto close_arr = arrow::MakeArrayFromScalar(arrow::DoubleScalar(), 0);
    HKU_ASSERT(close_arr.ok());
    auto amount_arr = arrow::MakeArrayFromScalar(arrow::DoubleScalar(), 0);
    HKU_ASSERT(amount_arr.ok());
    auto volume_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(volume_arr.ok());
    auto yesterday_close_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(yesterday_close_arr.ok());
    auto turnover_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(turnover_arr.ok());
    auto amplitude_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(amplitude_arr.ok());
    auto price_change_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(price_change_arr.ok());
    auto total_market_cap_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(total_market_cap_arr.ok());
    auto circulating_market_cap_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(circulating_market_cap_arr.ok());
    auto pe_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(pe_arr.ok());
    auto pb_arr = arrow::MakeArrayFromScalar(arrow::StringScalar(), 0);
    HKU_ASSERT(pb_arr.ok());

    return arrow::Table::Make(
      schema,
      {*code_arr, *name_arr, *date_arr, *open_arr, *high_arr, *low_arr, *close_arr, *amount_arr,
       *volume_arr, *yesterday_close_arr, *turnover_arr, *amplitude_arr, *price_change_arr,
       *total_market_cap_arr, *circulating_market_cap_arr, *pe_arr, *pb_arr});
}

std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks, const Datetime& date,
                                                    const string& market) {
    std::shared_ptr<arrow::Table> ret = crtEmtpyMarketView();
    auto& sm = StockManager::instance();

    Stock market_stk = sm.getMarketStock(market);
    HKU_ERROR_IF_RETURN(market_stk.isNull(), ret, "Can not find market stock for {}!", market);

    size_t start_pos, end_pos;
    HKU_ERROR_IF_RETURN(!market_stk.getIndexRange(KQueryByDate(date.startOfDay(), date.nextDay()),
                                                  start_pos, end_pos),
                        ret, "Unknown error!");

    HKU_ERROR_IF_RETURN(start_pos < 1, ret, "At least 2 trading days are required!");

    DatetimeList dates = market_stk.getDatetimeList(KQueryByIndex(start_pos - 1, start_pos + 1));
    HKU_ERROR_IF_RETURN(dates.size() < 2, ret, "At least 2 trading days are required!");

    Datetime yesterday = dates[0];
    Datetime today = dates[1];
    Datetime tomorrow = today.nextDay();

    Indicator EPS = FINANCE(0);
    EPS.setParam("dynamic", true);
    Indicator PE = CLOSE() / EPS;
    Indicator PB = CLOSE() / FINANCE(3);  // 市净率

    arrow::StringBuilder code_builder, name_builder;
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder open_builder, high_builder, low_builder, close_builder, amount_builder,
      volume_builder, yesterday_close_builder, turnover_builder, amplitude_builder,
      price_change_builder, total_market_cap_builder, circulating_market_cap_builder, pe_builder,
      pb_builder;

    size_t total = stks.size();
    HKU_ASSERT(code_builder.Reserve(total).ok());
    HKU_ASSERT(name_builder.Reserve(total).ok());
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(open_builder.Reserve(total).ok());
    HKU_ASSERT(high_builder.Reserve(total).ok());
    HKU_ASSERT(low_builder.Reserve(total).ok());
    HKU_ASSERT(close_builder.Reserve(total).ok());
    HKU_ASSERT(amount_builder.Reserve(total).ok());
    HKU_ASSERT(volume_builder.Reserve(total).ok());
    HKU_ASSERT(yesterday_close_builder.Reserve(total).ok());
    HKU_ASSERT(turnover_builder.Reserve(total).ok());
    HKU_ASSERT(amplitude_builder.Reserve(total).ok());
    HKU_ASSERT(price_change_builder.Reserve(total).ok());
    HKU_ASSERT(total_market_cap_builder.Reserve(total).ok());
    HKU_ASSERT(circulating_market_cap_builder.Reserve(total).ok());
    HKU_ASSERT(pe_builder.Reserve(total).ok());
    HKU_ASSERT(pb_builder.Reserve(total).ok());

    for (size_t i = 0; i < total; i++) {
        const Stock& stk = stks[i];
        if (!stk.isNull()) {
            auto kdata = stk.getKData(KQueryByDate(yesterday, tomorrow));
            if (kdata.size() == 2) {
                HKU_ASSERT(code_builder.Append(stk.market_code()).ok());
                HKU_ASSERT(name_builder.Append(stk.name()).ok());
                HKU_ASSERT(date_builder.Append(kdata[1].datetime.timestamp() * 1000LL).ok());
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
                                       ? roundEx(((kdata[1].closePrice - kdata[0].closePrice) /
                                                  kdata[0].closePrice) *
                                                   100.0,
                                                 2)
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
                HKU_ASSERT(price_change_builder.Append(Null<price_t>()).ok());
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

    HKU_ASSERT(code_builder.Finish(&code_arr).ok());
    HKU_ASSERT(name_builder.Finish(&name_arr).ok());
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(open_builder.Finish(&open_arr).ok());
    HKU_ASSERT(high_builder.Finish(&high_arr).ok());
    HKU_ASSERT(low_builder.Finish(&low_arr).ok());
    HKU_ASSERT(close_builder.Finish(&close_arr).ok());
    HKU_ASSERT(amount_builder.Finish(&amount_arr).ok());
    HKU_ASSERT(volume_builder.Finish(&volume_arr).ok());
    HKU_ASSERT(yesterday_close_builder.Finish(&yesterday_close_arr).ok());
    HKU_ASSERT(turnover_builder.Finish(&turnover_arr).ok());
    HKU_ASSERT(amplitude_builder.Finish(&amplitude_arr).ok());
    HKU_ASSERT(price_change_builder.Finish(&price_change_arr).ok());
    HKU_ASSERT(total_market_cap_builder.Finish(&total_market_cap_arr).ok());
    HKU_ASSERT(circulating_market_cap_builder.Finish(&circulating_market_cap_arr).ok());
    HKU_ASSERT(pe_builder.Finish(&pe_arr).ok());
    HKU_ASSERT(pb_builder.Finish(&pb_arr).ok());

    return arrow::Table::Make(
      ret->schema(),
      {code_arr, name_arr, date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr, volume_arr,
       yesterday_close_arr, turnover_arr, amplitude_arr, price_change_arr, total_market_cap_arr,
       circulating_market_cap_arr, pe_arr, pb_arr});
}

std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks, const string& market) {
    std::shared_ptr<arrow::Table> ret = crtEmtpyMarketView();
    DatetimeList dates = StockManager::instance().getTradingCalendar(KQueryByIndex(-1), market);
    HKU_IF_RETURN(dates.empty(), ret);

    return getMarketView(stks, dates.back(), market);
}

std::shared_ptr<arrow::Table> HKU_API getIndicatorsView(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const KQuery& query, const string& market,
                                                        bool parallel) {
    // SPEND_TIME(getIndicatorsView);
    auto& sm = StockManager::instance();
    auto dates = sm.getTradingCalendar(query, market);

    std::vector<std::shared_ptr<arrow::Field>> fields;
    fields.reserve(inds.size() + 3);
    fields.emplace_back(arrow::field(htr("market_code"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("stock_name"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("datetime"), arrow::timestamp(arrow::TimeUnit::NANO)));
    for (auto& ind : inds) {
        fields.emplace_back(arrow::field(ind.name(), HKU_ARROW_PRICE_FIELD));
    }

    arrow::StringBuilder code_builder, name_builder;
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    std::vector<HKU_ARROW_PRICE_BUILDER> builders(inds.size());

    size_t total = stks.size() * dates.size();
    HKU_ASSERT(code_builder.Reserve(total).ok());
    HKU_ASSERT(name_builder.Reserve(total).ok());
    HKU_ASSERT(date_builder.Reserve(total).ok());
    for (auto& builder : builders) {
        HKU_ASSERT(builder.Reserve(total).ok());
    }

    if (!dates.empty()) {
        KQuery new_query =
          KQueryByDate(dates.front(), dates.back() + Minutes(KQuery::getKTypeInMin(query.kType())),
                       query.kType(), query.recoverType());
        vector<int64_t> timestamps(dates.size());
        for (size_t i = 0; i < dates.size(); ++i) {
            timestamps[i] = dates[i].timestamp() * 1000LL;
        }

        if (parallel) {
            auto stk_ind_list = parallel_for_index(0, stks.size(), [&](size_t index) {
                auto kdata = stks[index].getKData(new_query);
                IndicatorList rets(inds.size());
                for (size_t i = 0; i < inds.size(); i++) {
                    rets[i] = ALIGN(inds[i].clone(), dates, true)(kdata);
                }
                return rets;
            });

            for (size_t i = 0; i < stk_ind_list.size(); i++) {
                const auto& stk_inds = stk_ind_list[i];
                for (size_t j = 0; j < stk_inds.size(); j++) {
                    if (stk_inds[j].size() == dates.size()) {
                        const auto* x_ptr = stk_inds[j].data();
                        HKU_ASSERT(builders[j].AppendValues(x_ptr, stk_inds[j].size()).ok());
                    } else {
                        HKU_ASSERT(builders[j].AppendNulls(dates.size()).ok());
                    }
                }
                for (size_t n = 0; n < dates.size(); ++n) {
                    HKU_ASSERT(code_builder.Append(stks[i].market_code()).ok());
                    HKU_ASSERT(name_builder.Append(stks[i].name()).ok());
                }
                HKU_ASSERT(date_builder.AppendValues(timestamps).ok());
            }
        } else {
            for (const auto& stk : stks) {
                auto kdata = stk.getKData(new_query);
                if (kdata.empty()) {
                    continue;
                }
                for (size_t i = 0; i < inds.size(); ++i) {
                    auto x = ALIGN(inds[i], dates, true)(kdata);
                    if (x.size() != dates.size()) {
                        HKU_ASSERT(builders[i].AppendNulls(dates.size()).ok());
                    } else {
                        const auto* x_ptr = x.data();
                        HKU_ASSERT(builders[i].AppendValues(x_ptr, x.size()).ok());
                    }
                }

                for (size_t i = 0; i < dates.size(); ++i) {
                    HKU_ASSERT(code_builder.Append(stk.market_code()).ok());
                    HKU_ASSERT(name_builder.Append(stk.name()).ok());
                }
                HKU_ASSERT(date_builder.AppendValues(timestamps).ok());
            }
        }
    }

    vector<std::shared_ptr<arrow::Array>> arrs;
    arrs.reserve(inds.size() + 3);

    auto code_arr = code_builder.Finish();
    HKU_ASSERT(code_arr.ok());
    arrs.push_back(*code_arr);

    auto name_arr = name_builder.Finish();
    HKU_ASSERT(name_arr.ok());
    arrs.push_back(*name_arr);

    auto date_arr = date_builder.Finish();
    HKU_ASSERT(date_arr.ok());
    arrs.push_back(*date_arr);

    for (size_t i = 0; i < inds.size(); ++i) {
        auto arr = builders[i].Finish();
        HKU_ASSERT(arr.ok());
        arrs.push_back(*arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

std::shared_ptr<arrow::Table> HKU_API getIndicatorsView(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const Datetime& date, size_t cal_len,
                                                        const KQuery::KType& ktype,
                                                        const string& market, bool parallel) {
    // SPEND_TIME(getIndicatorsView);
    auto& sm = StockManager::instance();
    Stock market_stk = sm.getMarketStock(market);
    HKU_CHECK(!market_stk.isNull(), "Can not find market stock for {}!", market);

    std::vector<std::shared_ptr<arrow::Field>> fields;
    fields.reserve(inds.size() + 3);
    fields.emplace_back(arrow::field(htr("market_code"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("stock_name"), arrow::utf8()));
    fields.emplace_back(arrow::field(htr("datetime"), arrow::timestamp(arrow::TimeUnit::NANO)));
    for (auto& ind : inds) {
        fields.emplace_back(arrow::field(ind.name(), HKU_ARROW_PRICE_FIELD));
    }

    arrow::StringBuilder code_builder, name_builder;
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
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

            if (parallel) {
                auto stk_ind_list = parallel_for_index(0, stks.size(), [&](size_t index) {
                    auto kdata = stks[index].getKData(query);
                    vector<Indicator::value_t> rets(inds.size());
                    for (size_t i = 0; i < inds.size(); i++) {
                        auto x = ALIGN(inds[i].clone(), dates, true)(kdata);
                        if (x.size() == dates.size()) {
                            rets[i] = x[x.size() - 1];
                        } else {
                            rets[i] = Null<Indicator::value_t>();
                        }
                    }
                    return rets;
                });

                for (size_t i = 0; i < stks.size(); ++i) {
                    for (size_t j = 0; j < inds.size(); ++j) {
                        HKU_ASSERT(builders[j].Append(stk_ind_list[i][j]).ok());
                    }
                    HKU_ASSERT(code_builder.Append(stks[i].market_code()).ok());
                    HKU_ASSERT(name_builder.Append(stks[i].name()).ok());
                    HKU_ASSERT(date_builder.Append(dates.back().timestamp() * 1000LL).ok());
                }

            } else {
                for (const auto& stk : stks) {
                    auto kdata = stk.getKData(query);
                    for (size_t i = 0; i < inds.size(); ++i) {
                        auto x = ALIGN(inds[i], dates, true)(kdata);
                        if (x.size() != dates.size()) {
                            HKU_ASSERT(builders[i].AppendNull().ok());
                        } else {
                            HKU_ASSERT(builders[i].Append(x[dates.size() - 1]).ok());
                        }
                    }

                    HKU_ASSERT(code_builder.Append(stk.market_code()).ok());
                    HKU_ASSERT(name_builder.Append(stk.name()).ok());
                    HKU_ASSERT(date_builder.Append(dates.back().timestamp() * 1000LL).ok());
                }
            }
        }
    }

    vector<std::shared_ptr<arrow::Array>> arrs;
    arrs.reserve(inds.size() + 3);

    auto code_arr = code_builder.Finish();
    HKU_ASSERT(code_arr.ok());
    arrs.push_back(*code_arr);

    auto name_arr = name_builder.Finish();
    HKU_ASSERT(name_arr.ok());
    arrs.push_back(*name_arr);

    auto date_arr = date_builder.Finish();
    HKU_ASSERT(date_arr.ok());
    arrs.push_back(*date_arr);

    for (size_t i = 0; i < inds.size(); ++i) {
        auto arr = builders[i].Finish();
        HKU_ASSERT(arr.ok());
        arrs.push_back(*arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

std::shared_ptr<arrow::Table> HKU_API getIndicatorView(const Indicator& ind) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    vector<HKU_ARROW_PRICE_BUILDER> value_builders;

    size_t result_num = ind.getResultNumber();
    for (size_t i = 0; i < result_num; i++) {
        value_builders.emplace_back();
    }

    size_t total = ind.size();
    auto dates = ind.getDatetimeList();
    if (!dates.empty()) {
        for (size_t i = 0; i < total; i++) {
            HKU_ASSERT(date_builder.Append(dates[i].timestamp() * 1000LL).ok());
        }
    }

    for (size_t r = 0; r < result_num; r++) {
        HKU_ASSERT(value_builders[r].AppendValues(ind.data(r), total).ok());
    }

    std::vector<std::shared_ptr<arrow::Field>> fields;
    vector<std::shared_ptr<arrow::Array>> arrs;

    if (!dates.empty()) {
        fields.emplace_back(arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)));
        auto date_arr = date_builder.Finish();
        HKU_ASSERT(date_arr.ok());
        arrs.push_back(*date_arr);
    }

    for (size_t r = 0; r < result_num; r++) {
        fields.emplace_back(arrow::field(fmt::format("value{}", r + 1), HKU_ARROW_PRICE_FIELD));
        auto value_arr = value_builders[r].Finish();
        HKU_ASSERT(value_arr.ok());
        arrs.push_back(*value_arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

std::shared_ptr<arrow::Table> HKU_API getIndicatorValueView(const Indicator& ind) {
    vector<HKU_ARROW_PRICE_BUILDER> value_builders;

    size_t result_num = ind.getResultNumber();
    for (size_t i = 0; i < result_num; i++) {
        value_builders.emplace_back();
    }

    size_t total = ind.size();
    for (size_t r = 0; r < result_num; r++) {
        HKU_ASSERT(value_builders[r].AppendValues(ind.data(r), total).ok());
    }

    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (size_t r = 0; r < result_num; r++) {
        fields.emplace_back(arrow::field(fmt::format("value{}", r + 1), HKU_ARROW_PRICE_FIELD));
    }

    vector<std::shared_ptr<arrow::Array>> arrs;
    for (size_t r = 0; r < result_num; r++) {
        auto value_arr = value_builders[r].Finish();
        HKU_ASSERT(value_arr.ok());
        arrs.push_back(*value_arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

std::shared_ptr<arrow::Table> HKU_API getKDataView(const KData& kdata) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder open_builder, high_builder, low_builder, close_builder, amount_builder,
      volume_builder;
    size_t total = kdata.size();
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(open_builder.Reserve(total).ok());
    HKU_ASSERT(high_builder.Reserve(total).ok());
    HKU_ASSERT(low_builder.Reserve(total).ok());
    HKU_ASSERT(close_builder.Reserve(total).ok());
    HKU_ASSERT(amount_builder.Reserve(total).ok());
    HKU_ASSERT(volume_builder.Reserve(total).ok());

    const auto* ks = kdata.data();
    for (size_t i = 0; i < total; ++i) {
        HKU_ASSERT(date_builder.Append(ks[i].datetime.timestamp() * 1000LL).ok());
        HKU_ASSERT(open_builder.Append(ks[i].openPrice).ok());
        HKU_ASSERT(high_builder.Append(ks[i].highPrice).ok());
        HKU_ASSERT(low_builder.Append(ks[i].lowPrice).ok());
        HKU_ASSERT(close_builder.Append(ks[i].closePrice).ok());
        HKU_ASSERT(amount_builder.Append(ks[i].transAmount).ok());
        HKU_ASSERT(volume_builder.Append(ks[i].transCount).ok());
    }

    std::shared_ptr<arrow::Array> date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr,
      volume_arr;
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(open_builder.Finish(&open_arr).ok());
    HKU_ASSERT(high_builder.Finish(&high_arr).ok());
    HKU_ASSERT(low_builder.Finish(&low_arr).ok());
    HKU_ASSERT(close_builder.Finish(&close_arr).ok());
    HKU_ASSERT(amount_builder.Finish(&amount_arr).ok());
    HKU_ASSERT(volume_builder.Finish(&volume_arr).ok());

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)),
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

std::shared_ptr<arrow::Table> HKU_API getKRecordListView(const KRecordList& ks) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder open_builder, high_builder, low_builder, close_builder, amount_builder,
      volume_builder;
    size_t total = ks.size();
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(open_builder.Reserve(total).ok());
    HKU_ASSERT(high_builder.Reserve(total).ok());
    HKU_ASSERT(low_builder.Reserve(total).ok());
    HKU_ASSERT(close_builder.Reserve(total).ok());
    HKU_ASSERT(amount_builder.Reserve(total).ok());
    HKU_ASSERT(volume_builder.Reserve(total).ok());

    for (auto& k : ks) {
        HKU_ASSERT(date_builder.Append(k.datetime.timestamp() * 1000LL).ok());
        HKU_ASSERT(open_builder.Append(k.openPrice).ok());
        HKU_ASSERT(high_builder.Append(k.highPrice).ok());
        HKU_ASSERT(low_builder.Append(k.lowPrice).ok());
        HKU_ASSERT(close_builder.Append(k.closePrice).ok());
        HKU_ASSERT(amount_builder.Append(k.transAmount).ok());
        HKU_ASSERT(volume_builder.Append(k.transCount).ok());
    }

    std::shared_ptr<arrow::Array> date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr,
      volume_arr;
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(open_builder.Finish(&open_arr).ok());
    HKU_ASSERT(high_builder.Finish(&high_arr).ok());
    HKU_ASSERT(low_builder.Finish(&low_arr).ok());
    HKU_ASSERT(close_builder.Finish(&close_arr).ok());
    HKU_ASSERT(amount_builder.Finish(&amount_arr).ok());
    HKU_ASSERT(volume_builder.Finish(&volume_arr).ok());

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)),
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

std::shared_ptr<arrow::Table> HKU_API getTimeLineListView(const TimeLineList& ts) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder price_builder, vol_builder;
    size_t total = ts.size();
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(price_builder.Reserve(total).ok());
    HKU_ASSERT(vol_builder.Reserve(total).ok());

    for (auto& t : ts) {
        HKU_ASSERT(date_builder.Append(t.datetime.timestamp() * 1000LL).ok());
        HKU_ASSERT(price_builder.Append(t.price).ok());
        HKU_ASSERT(vol_builder.Append(t.vol).ok());
    }

    std::shared_ptr<arrow::Array> date_arr, price_arr, vol_arr;
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(price_builder.Finish(&price_arr).ok());
    HKU_ASSERT(vol_builder.Finish(&vol_arr).ok());

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)),
      arrow::field("price", arrow::float64()),
      arrow::field("vol", arrow::float64()),
    });

    return arrow::Table::Make(schema, {date_arr, price_arr, vol_arr});
}

std::shared_ptr<arrow::Table> HKU_API getTransRecordListView(const TransRecordList& ts) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder price_builder, vol_builder;
    arrow::Int32Builder direct_builder;
    size_t total = ts.size();
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(price_builder.Reserve(total).ok());
    HKU_ASSERT(vol_builder.Reserve(total).ok());
    HKU_ASSERT(direct_builder.Reserve(total).ok());

    for (auto& t : ts) {
        HKU_ASSERT(date_builder.Append(t.datetime.timestamp() * 1000LL).ok());
        HKU_ASSERT(price_builder.Append(t.price).ok());
        HKU_ASSERT(vol_builder.Append(t.vol).ok());
        HKU_ASSERT(direct_builder.Append(t.direct).ok());
    }

    std::shared_ptr<arrow::Array> date_arr, price_arr, vol_arr, direct_arr;
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(price_builder.Finish(&price_arr).ok());
    HKU_ASSERT(vol_builder.Finish(&vol_arr).ok());
    HKU_ASSERT(direct_builder.Finish(&direct_arr).ok());

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)),
      arrow::field("price", arrow::float64()),
      arrow::field("vol", arrow::float64()),
      arrow::field("direct", arrow::int32()),
    });

    return arrow::Table::Make(schema, {date_arr, price_arr, vol_arr, direct_arr});
}

std::shared_ptr<arrow::Table> HKU_API getStockWeightListView(const StockWeightList& sws) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder countAsGift_builder, countForSell_builder, priceForSell_builder,
      bonus_builder, increasement_builder, totalCount_builder, freeCount_builder, suogu_builder;
    size_t total = sws.size();
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(countAsGift_builder.Reserve(total).ok());
    HKU_ASSERT(countForSell_builder.Reserve(total).ok());
    HKU_ASSERT(priceForSell_builder.Reserve(total).ok());
    HKU_ASSERT(bonus_builder.Reserve(total).ok());
    HKU_ASSERT(increasement_builder.Reserve(total).ok());
    HKU_ASSERT(totalCount_builder.Reserve(total).ok());
    HKU_ASSERT(freeCount_builder.Reserve(total).ok());
    HKU_ASSERT(suogu_builder.Reserve(total).ok());

    for (auto& w : sws) {
        HKU_ASSERT(date_builder.Append(w.datetime().timestamp() * 1000LL).ok());
        HKU_ASSERT(countAsGift_builder.Append(w.countAsGift()).ok());
        HKU_ASSERT(countForSell_builder.Append(w.countForSell()).ok());
        HKU_ASSERT(priceForSell_builder.Append(w.priceForSell()).ok());
        HKU_ASSERT(bonus_builder.Append(w.bonus()).ok());
        HKU_ASSERT(increasement_builder.Append(w.increasement()).ok());
        HKU_ASSERT(totalCount_builder.Append(w.totalCount()).ok());
        HKU_ASSERT(freeCount_builder.Append(w.freeCount()).ok());
        HKU_ASSERT(suogu_builder.Append(w.suogu()).ok());
    }

    std::shared_ptr<arrow::Array> date_arr, countAsGift_arr, countForSell_arr, priceForSell_arr,
      bonus_arr, increasement_arr, totalCount_arr, freeCount_arr, suogu_arr;
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(countAsGift_builder.Finish(&countAsGift_arr).ok());
    HKU_ASSERT(countForSell_builder.Finish(&countForSell_arr).ok());
    HKU_ASSERT(priceForSell_builder.Finish(&priceForSell_arr).ok());
    HKU_ASSERT(bonus_builder.Finish(&bonus_arr).ok());
    HKU_ASSERT(increasement_builder.Finish(&increasement_arr).ok());
    HKU_ASSERT(totalCount_builder.Finish(&totalCount_arr).ok());
    HKU_ASSERT(freeCount_builder.Finish(&freeCount_arr).ok());
    HKU_ASSERT(suogu_builder.Finish(&suogu_arr).ok());

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)),
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

std::shared_ptr<arrow::Table> HKU_API getDatetimeListView(const DatetimeList& dates) {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    HKU_ASSERT(date_builder.Reserve(dates.size()).ok());

    for (auto& date : dates) {
        HKU_ASSERT(date_builder.Append(date.timestamp() * 1000LL).ok());
    }

    std::shared_ptr<arrow::Array> date_arr;
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());

    auto schema =
      arrow::schema({arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO))});
    return arrow::Table::Make(schema, {date_arr});
}

std::shared_ptr<arrow::Table> HKU_API getTradeRecordListView(const TradeRecordList& trades) {
    arrow::StringBuilder code_builder, name_builder, business_builder, sig_builder, remark_builder;
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder plan_builder, real_builder, goal_builder, stoploss_builder, number_builder,
      cash_builder, cost_total_builder, cost_commission_builder, cost_stamptax_builder,
      cost_transferfee_builder, cost_other_builder;

    size_t total = trades.size();
    HKU_ASSERT(code_builder.Reserve(total).ok());
    HKU_ASSERT(name_builder.Reserve(total).ok());
    HKU_ASSERT(date_builder.Reserve(total).ok());
    HKU_ASSERT(business_builder.Reserve(total).ok());
    HKU_ASSERT(plan_builder.Reserve(total).ok());
    HKU_ASSERT(real_builder.Reserve(total).ok());
    HKU_ASSERT(goal_builder.Reserve(total).ok());
    HKU_ASSERT(number_builder.Reserve(total).ok());
    HKU_ASSERT(stoploss_builder.Reserve(total).ok());
    HKU_ASSERT(cash_builder.Reserve(total).ok());
    HKU_ASSERT(cost_total_builder.Reserve(total).ok());
    HKU_ASSERT(cost_commission_builder.Reserve(total).ok());
    HKU_ASSERT(cost_stamptax_builder.Reserve(total).ok());
    HKU_ASSERT(cost_transferfee_builder.Reserve(total).ok());
    HKU_ASSERT(cost_other_builder.Reserve(total).ok());
    HKU_ASSERT(sig_builder.Reserve(total).ok());
    HKU_ASSERT(remark_builder.Reserve(total).ok());

    for (auto& tr : trades) {
        HKU_ASSERT(code_builder.Append(tr.stock.market_code()).ok());
        HKU_ASSERT(name_builder.Append(tr.stock.name()).ok());
        HKU_ASSERT(date_builder.Append(tr.datetime.timestamp() * 1000LL).ok());
        HKU_ASSERT(business_builder.Append(getBusinessName(tr.business)).ok());
        HKU_ASSERT(plan_builder.Append(tr.planPrice).ok());
        HKU_ASSERT(real_builder.Append(tr.realPrice).ok());
        HKU_ASSERT(goal_builder.Append(tr.goalPrice).ok());
        HKU_ASSERT(number_builder.Append(tr.number).ok());
        HKU_ASSERT(stoploss_builder.Append(tr.stoploss).ok());
        HKU_ASSERT(cash_builder.Append(tr.cash).ok());
        HKU_ASSERT(cost_total_builder.Append(tr.cost.total).ok());
        HKU_ASSERT(cost_commission_builder.Append(tr.cost.commission).ok());
        HKU_ASSERT(cost_stamptax_builder.Append(tr.cost.stamptax).ok());
        HKU_ASSERT(cost_transferfee_builder.Append(tr.cost.transferfee).ok());
        HKU_ASSERT(cost_other_builder.Append(tr.cost.others).ok());
        HKU_ASSERT(sig_builder.Append(getSystemPartName(tr.from)).ok());
        HKU_ASSERT(remark_builder.Append(tr.remark).ok());
    }

    std::shared_ptr<arrow::Array> code_arr, name_arr, date_arr, business_arr, plan_arr, real_arr,
      goal_arr, number_arr, stoploss_arr, cash_arr, cost_total_arr, cost_commission_arr,
      cost_stamptax_arr, cost_transferfee_arr, cost_other_arr, sig_arr, remark_arr;

    HKU_ASSERT(code_builder.Finish(&code_arr).ok());
    HKU_ASSERT(name_builder.Finish(&name_arr).ok());
    HKU_ASSERT(date_builder.Finish(&date_arr).ok());
    HKU_ASSERT(business_builder.Finish(&business_arr).ok());
    HKU_ASSERT(plan_builder.Finish(&plan_arr).ok());
    HKU_ASSERT(real_builder.Finish(&real_arr).ok());
    HKU_ASSERT(goal_builder.Finish(&goal_arr).ok());
    HKU_ASSERT(number_builder.Finish(&number_arr).ok());
    HKU_ASSERT(stoploss_builder.Finish(&stoploss_arr).ok());
    HKU_ASSERT(cash_builder.Finish(&cash_arr).ok());
    HKU_ASSERT(cost_total_builder.Finish(&cost_total_arr).ok());
    HKU_ASSERT(cost_commission_builder.Finish(&cost_commission_arr).ok());
    HKU_ASSERT(cost_stamptax_builder.Finish(&cost_stamptax_arr).ok());
    HKU_ASSERT(cost_transferfee_builder.Finish(&cost_transferfee_arr).ok());
    HKU_ASSERT(cost_other_builder.Finish(&cost_other_arr).ok());
    HKU_ASSERT(sig_builder.Finish(&sig_arr).ok());
    HKU_ASSERT(remark_builder.Finish(&remark_arr).ok());

    auto schema = arrow::schema({
      arrow::field(htr("market_code"), arrow::utf8()),
      arrow::field(htr("stock_name"), arrow::utf8()),
      arrow::field(htr("datetime"), arrow::timestamp(arrow::TimeUnit::NANO)),
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

std::shared_ptr<arrow::Table> HKU_API
getPositionRecordListView(const PositionRecordList& positions) {
    arrow::StringBuilder code_builder, name_builder;
    arrow::TimestampBuilder taketime_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                             arrow::default_memory_pool());
    arrow::TimestampBuilder cleantime_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                              arrow::default_memory_pool());
    arrow::Int64Builder hold_days_builder;
    arrow::DoubleBuilder hold_number_builder, invest_builder, market_value_builder, profit_builder,
      profit_percent_builder, stoploss_builder, goal_price_builder, total_number_builder,
      total_cost_builder, total_risk_builder, buy_money_builder, sell_money_builder;

    size_t total = positions.size();
    HKU_ASSERT(code_builder.Reserve(total).ok());
    HKU_ASSERT(name_builder.Reserve(total).ok());
    HKU_ASSERT(taketime_builder.Reserve(total).ok());
    HKU_ASSERT(cleantime_builder.Reserve(total).ok());
    HKU_ASSERT(hold_days_builder.Reserve(total).ok());
    HKU_ASSERT(hold_number_builder.Reserve(total).ok());
    HKU_ASSERT(invest_builder.Reserve(total).ok());
    HKU_ASSERT(market_value_builder.Reserve(total).ok());
    HKU_ASSERT(profit_builder.Reserve(total).ok());
    HKU_ASSERT(profit_percent_builder.Reserve(total).ok());
    HKU_ASSERT(stoploss_builder.Reserve(total).ok());
    HKU_ASSERT(goal_price_builder.Reserve(total).ok());
    HKU_ASSERT(total_number_builder.Reserve(total).ok());
    HKU_ASSERT(total_cost_builder.Reserve(total).ok());
    HKU_ASSERT(total_risk_builder.Reserve(total).ok());
    HKU_ASSERT(buy_money_builder.Reserve(total).ok());
    HKU_ASSERT(sell_money_builder.Reserve(total).ok());

    for (const auto& p : positions) {
        HKU_ASSERT(code_builder.Append(p.stock.market_code()).ok());
        HKU_ASSERT(name_builder.Append(p.stock.name()).ok());
        HKU_ASSERT(taketime_builder.Append(p.takeDatetime.timestamp() * 1000LL).ok());

        double invest = p.buyMoney - p.sellMoney + p.totalCost;
        double profit = 0.0;
        if (p.cleanDatetime.isNull()) {
            HKU_ASSERT(cleantime_builder.AppendNull().ok());
            double market_value = p.stock.getMarketValue(Datetime::now(), KQuery::DAY) * p.number;
            HKU_ASSERT(hold_days_builder.Append((Datetime::now() - p.takeDatetime).days()).ok());
            HKU_ASSERT(market_value_builder.Append(market_value).ok());
            profit = market_value - invest;
            HKU_ASSERT(profit_builder.Append(profit).ok());
        } else {
            HKU_ASSERT(cleantime_builder.Append(p.cleanDatetime.timestamp() * 1000LL).ok());
            HKU_ASSERT(hold_days_builder.Append((p.cleanDatetime - p.takeDatetime).days()).ok());
            HKU_ASSERT(market_value_builder.Append(0.0).ok());
            profit = p.totalProfit();
            HKU_ASSERT(profit_builder.Append(profit).ok());
        }
        HKU_ASSERT(hold_number_builder.Append(p.number).ok());
        HKU_ASSERT(invest_builder.Append(invest).ok());
        HKU_ASSERT(
          profit_percent_builder.Append(invest != 0.0 ? roundEx(100. * (profit / invest), 2) : 0.0)
            .ok());
        HKU_ASSERT(stoploss_builder.Append(p.stoploss).ok());
        HKU_ASSERT(goal_price_builder.Append(p.goalPrice).ok());
        HKU_ASSERT(total_number_builder.Append(p.totalNumber).ok());
        HKU_ASSERT(total_cost_builder.Append(p.totalCost).ok());
        HKU_ASSERT(total_risk_builder.Append(p.totalRisk).ok());
        HKU_ASSERT(buy_money_builder.Append(p.buyMoney).ok());
        HKU_ASSERT(sell_money_builder.Append(p.sellMoney).ok());
    }

    std::shared_ptr<arrow::Array> code_arr, name_arr, taketime_arr, cleantime_arr, hold_days_arr,
      hold_number_arr, invest_arr, market_value_arr, profit_arr, profit_percent_arr, stoploss_arr,
      goal_price_arr, total_number_arr, total_cost_arr, total_risk_arr, buy_money_arr,
      sell_money_arr;

    HKU_ASSERT(code_builder.Finish(&code_arr).ok());
    HKU_ASSERT(name_builder.Finish(&name_arr).ok());
    HKU_ASSERT(taketime_builder.Finish(&name_arr).ok());
    HKU_ASSERT(cleantime_builder.Finish(&cleantime_arr).ok());
    HKU_ASSERT(hold_days_builder.Finish(&hold_days_arr).ok());
    HKU_ASSERT(hold_number_builder.Finish(&hold_number_arr).ok());
    HKU_ASSERT(invest_builder.Finish(&invest_arr).ok());
    HKU_ASSERT(market_value_builder.Finish(&market_value_arr).ok());
    HKU_ASSERT(profit_builder.Finish(&profit_arr).ok());
    HKU_ASSERT(profit_percent_builder.Finish(&profit_percent_arr).ok());
    HKU_ASSERT(stoploss_builder.Finish(&stoploss_arr).ok());
    HKU_ASSERT(goal_price_builder.Finish(&goal_price_arr).ok());
    HKU_ASSERT(total_number_builder.Finish(&total_number_arr).ok());
    HKU_ASSERT(total_cost_builder.Finish(&total_cost_arr).ok());
    HKU_ASSERT(total_risk_builder.Finish(&total_risk_arr).ok());
    HKU_ASSERT(buy_money_builder.Finish(&buy_money_arr).ok());
    HKU_ASSERT(sell_money_builder.Finish(&sell_money_arr).ok());

    auto schema = arrow::schema({
      arrow::field(htr("market_code"), arrow::utf8()),
      arrow::field(htr("stock_name"), arrow::utf8()),
      arrow::field(htr("take_time"), arrow::timestamp(arrow::TimeUnit::NANO)),
      arrow::field(htr("hold_days"), arrow::int64()),
      arrow::field(htr("hold_number"), arrow::date64()),
      arrow::field(htr("invest"), arrow::float64()),
      arrow::field(htr("market_value"), arrow::float64()),
      arrow::field(htr("profit"), arrow::float64()),
      arrow::field(htr("profit_percent"), arrow::float64()),
      arrow::field(htr("stoploss"), arrow::float64()),
      arrow::field(htr("goal_price"), arrow::float64()),
      arrow::field(htr("clean_time"), arrow::timestamp(arrow::TimeUnit::NANO)),
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