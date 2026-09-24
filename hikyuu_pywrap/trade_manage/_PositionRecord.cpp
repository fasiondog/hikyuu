/*
 * _PositionRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/PositionRecord.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_PositionRecord(py::module& m) {
    py::class_<PositionRecord>(m, "PositionRecord", "The position record")
      .def(py::init<>())
      .def(py::init<const Stock&, const Datetime&, const Datetime&, double, price_t, price_t,
                    double, price_t, price_t, price_t, price_t>())

      .def("__str__", &PositionRecord::str)
      .def("__repr__", &PositionRecord::str)

      .def_readwrite("stock", &PositionRecord::stock, "The trading object (Stock)")
      .def_readwrite("take_datetime", &PositionRecord::takeDatetime, "The initial position building moment (Datetime)")
      .def_readwrite("clean_datetime", &PositionRecord::cleanDatetime,
                     "The closing date; in the current position records it is constant.null_datetime")
      .def_readwrite("number", &PositionRecord::number, "The current position quantity (float)")
      .def_readwrite("stoploss", &PositionRecord::stoploss, "The current stop-loss price (float)")
      .def_readwrite("goal_price", &PositionRecord::goalPrice, "The current target price (float)")
      .def_readwrite("total_number", &PositionRecord::totalNumber, "The cumulative position quantity (float)")
      .def_readwrite("buy_money", &PositionRecord::buyMoney, "The cumulative buy funds (float)")
      .def_readwrite("total_cost", &PositionRecord::totalCost, "The cumulative total trading cost (float)")
      .def_readwrite("total_risk", &PositionRecord::totalRisk,
                     "The cumulative trading risk = each (the buy price - the stop-loss) * the buy quantity, excluding the trading costs")
      .def_readwrite("sell_money", &PositionRecord::sellMoney, "The cumulative sell funds (float)")
      .def_readwrite("buy_count", &PositionRecord::buyCount, "The cumulative buy count (size_t)")
      .def_readwrite("sell_count", &PositionRecord::sellCount, "The cumulative sell count (size_t)")
      .def_property_readonly("total_profit", &PositionRecord::totalProfit,
                             R"(total_profit(self):

    The cumulative profit = the cumulative sell funds - the cumulative buy funds - the cumulative trading cost
    Note: it is only valid for the closed records; the open records return 0  )")

        DEF_PICKLE(PositionRecord);

    py::class_<PositionExtInfo>(m, "PositionExtInfo", "The extended position information")
      .def(py::init<>())
      .def_readwrite("position", &PositionExtInfo::position, "PositionRecord")
      .def_readwrite("current_close_price", &PositionExtInfo::currentClosePrice,
                     "The current close price (float)")
      .def_readwrite("max_high_price", &PositionExtInfo::maxHighPrice, "The maximum of the highest prices in the period")
      .def_readwrite("min_low_price", &PositionExtInfo::minLowPrice, "The minimum of the lowest prices in the period")
      .def_readwrite("max_close_price", &PositionExtInfo::maxClosePrice, "The highest close price in the period")
      .def_readwrite("min_close_price", &PositionExtInfo::minClosePrice, "The lowest close price in the period")
      .def_readwrite("current_close_price", &PositionExtInfo::currentClosePrice, "The current close price")
      .def_readwrite("max_pull_back1", &PositionExtInfo::maxPullBack1,
                     "The maximum drawdown ratio 1 (calculated only with the maximum close price and the lowest close price) (a negative number)")
      .def_readwrite("max_pull_back2", &PositionExtInfo::maxPullBack2,
                     "The maximum drawdown ratio 2 (calculated with the maximum of the highest prices and the minimum of the lowest prices in the period) (a negative number)")
      .def_readwrite("current_profit", &PositionExtInfo::currentProfit,
                     "The current floating profit and loss (excluding the estimated sell cost)")

      .def("current_pull_back1", &PositionExtInfo::currentPullBack1,
           "The current drawdown percentage 1 (calculated only with the maximum close price and the current close price)")
      .def("current_pull_back2", &PositionExtInfo::currentPullBack2,
           "The current drawdown percentage 2 (calculated with the maximum of the highest prices in the period and the current close price)")
      .def("max_floating_profit1", &PositionExtInfo::maxFloatingProfit1,
           "The maximum floating profit 1 in the period (a positive number, calculated only with the close price, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)")
      .def("max_floating_profit2", &PositionExtInfo::maxFloatingProfit2,
           "The maximum floating profit 2 in the period (a positive number, calculated with the maximum of the highest prices, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)")
      .def("min_loss_profit1", &PositionExtInfo::minLossProfit1,
           "The maximum floating loss 1 in the period (a negative number, calculated only with the close price, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)")
      .def("min_loss_profit2", &PositionExtInfo::minLossProfit2,
           "The maximum floating loss 2 in the period (a negative number, calculated only with the lowest price in the period, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)")

        DEF_PICKLE(PositionExtInfo);

    m.def(
      "positions_to_np",
      [](const PositionRecordList& positions) {
          size_t total = positions.size();
          HKU_IF_RETURN(total == 0, py::array());

          struct alignas(8) RawData {
              int32_t code[10];
              int32_t name[20];
              int64_t take_datetime;        // The buy date
              int64_t hold_days;            // The held days
              double number;                // The current position quantity
              double invest;                // The current invested amount
              double current_market_value;  // The current market value
              double profit;                // The current profit/loss amount
              double profit_ratio;          // The current profit/loss ratio
              double stoploss;              // The current stop-loss price
              double goal_price;            // The current target price
              int64_t clean_datetime;       // The sell date
              double total_number;          // The cumulative position quantity
              double total_cost;            // The cumulative trading cost
              double total_risk;  // The cumulative trading risk = each (the buy price - the stop-loss) * the buy quantity, excluding the trading costs
              double buy_money;   // The cumulative invested amount
              double sell_money;  // The cumulative sell funds
          };

          RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
          for (size_t i = 0, total = positions.size(); i < total; i++) {
              const PositionRecord& p = positions[i];
              utf8_to_utf32(p.stock.market_code(), data[i].code, 10);
              utf8_to_utf32(p.stock.name(), data[i].name, 20);
              data[i].take_datetime = p.takeDatetime.timestamp() * 1000LL;
              data[i].number = p.number;
              data[i].invest = p.buyMoney - p.sellMoney + p.totalCost;
              if (p.cleanDatetime.isNull()) {
                  data[i].hold_days = (Datetime::now() - p.takeDatetime).days();
                  double cur_price = p.stock.getMarketValue(Datetime::now(), KQuery::DAY);
                  data[i].current_market_value = cur_price * p.number;
                  data[i].profit = data[i].current_market_value - data[i].invest;
              } else {
                  data[i].hold_days = (p.cleanDatetime - p.takeDatetime).days();
                  data[i].current_market_value = 0.0;
                  data[i].profit = p.totalProfit();
              }
              data[i].profit_ratio =
                roundEx(100. * (data[i].invest != 0.0 ? data[i].profit / data[i].invest : 0), 2);
              data[i].stoploss = p.stoploss;
              data[i].goal_price = p.goalPrice;
              data[i].clean_datetime = p.cleanDatetime.isNull()
                                         ? std::numeric_limits<int64_t>::min()
                                         : p.cleanDatetime.timestamp() * 1000LL;
              data[i].total_number = p.totalNumber;
              data[i].total_cost = p.totalCost;
              data[i].total_risk = p.totalRisk;
              data[i].buy_money = p.buyMoney;
              data[i].sell_money = p.sellMoney;
          }

          py::dtype dtype =
            py::dtype(vector_to_python_list<string>(
                        {htr("market_code"), htr("stock_name"), htr("take_time"), htr("hold_days"),
                         htr("hold_number"), htr("invest"), htr("market_value"), htr("profit"),
                         htr("profit_percent"), htr("stoploss"), htr("goal_price"),
                         htr("clean_time"), htr("total_number"), htr("total_cost"),
                         htr("total_risk"), htr("buy_money"), htr("sell_money")}),
                      vector_to_python_list<string>({"U10", "U20", "datetime64[ns]", "i8", "d", "d",
                                                     "d", "d", "d", "d", "d", "datetime64[ns]", "d",
                                                     "d", "d", "d", "d"}),
                      vector_to_python_list<int64_t>({0, 40, 120, 128, 136, 144, 152, 160, 168, 176,
                                                      184, 192, 200, 208, 216, 224, 232}),
                      240);

          return py::array(dtype, total, static_cast<RawData*>(data),
                           py::capsule(data, [](void* p) { std::free(p); }));
      },
      R"(Convert the position list to Numpy
    
    Note: the calculated values such as the current market value, the profit and the profit/loss are all calculated by the daily line; when backtesting with a level below the daily line, you need to recalculate the open position records yourself!)");

    m.def(
      "positions_to_df",
      [](const PositionRecordList& positions) {
          size_t total = positions.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // Create the data containers of each column
          py::list code_list(total);
          py::list name_list(total);
          py::array_t<int64_t> take_time_arr(total);
          py::array_t<int64_t> hold_days_arr(total);
          py::array_t<double> hold_number_arr(total);
          py::array_t<double> invest_arr(total);
          py::array_t<double> market_value_arr(total);
          py::array_t<double> profit_arr(total);
          py::array_t<double> profit_percent_arr(total);
          py::array_t<double> stoploss_arr(total);
          py::array_t<double> goal_price_arr(total);
          py::array_t<int64_t> clean_time_arr(total);
          py::array_t<double> total_number_arr(total);
          py::array_t<double> total_cost_arr(total);
          py::array_t<double> total_risk_arr(total);
          py::array_t<double> buy_money_arr(total);
          py::array_t<double> sell_money_arr(total);

          // Get the buffers of each array
          auto take_time_buf = take_time_arr.request();
          auto hold_days_buf = hold_days_arr.request();
          auto hold_number_buf = hold_number_arr.request();
          auto invest_buf = invest_arr.request();
          auto market_value_buf = market_value_arr.request();
          auto profit_buf = profit_arr.request();
          auto profit_percent_buf = profit_percent_arr.request();
          auto stoploss_buf = stoploss_arr.request();
          auto goal_price_buf = goal_price_arr.request();
          auto clean_time_buf = clean_time_arr.request();
          auto total_number_buf = total_number_arr.request();
          auto total_cost_buf = total_cost_arr.request();
          auto total_risk_buf = total_risk_arr.request();
          auto buy_money_buf = buy_money_arr.request();
          auto sell_money_buf = sell_money_arr.request();

          int64_t* take_time_ptr = static_cast<int64_t*>(take_time_buf.ptr);
          int64_t* hold_days_ptr = static_cast<int64_t*>(hold_days_buf.ptr);
          double* hold_number_ptr = static_cast<double*>(hold_number_buf.ptr);
          double* invest_ptr = static_cast<double*>(invest_buf.ptr);
          double* market_value_ptr = static_cast<double*>(market_value_buf.ptr);
          double* profit_ptr = static_cast<double*>(profit_buf.ptr);
          double* profit_percent_ptr = static_cast<double*>(profit_percent_buf.ptr);
          double* stoploss_ptr = static_cast<double*>(stoploss_buf.ptr);
          double* goal_price_ptr = static_cast<double*>(goal_price_buf.ptr);
          int64_t* clean_time_ptr = static_cast<int64_t*>(clean_time_buf.ptr);
          double* total_number_ptr = static_cast<double*>(total_number_buf.ptr);
          double* total_cost_ptr = static_cast<double*>(total_cost_buf.ptr);
          double* total_risk_ptr = static_cast<double*>(total_risk_buf.ptr);
          double* buy_money_ptr = static_cast<double*>(buy_money_buf.ptr);
          double* sell_money_ptr = static_cast<double*>(sell_money_buf.ptr);

          // Fill the data
          for (size_t i = 0; i < total; i++) {
              const PositionRecord& p = positions[i];
              if (!p.stock.isNull()) {
                  code_list[i] = py::str(p.stock.market_code());
                  name_list[i] = py::str(p.stock.name());
              } else {
                  code_list[i] = py::str("");
                  name_list[i] = py::str("");
              }

              take_time_ptr[i] = p.takeDatetime.timestamp() * 1000LL;

              int64_t hold_days;
              double current_market_value;
              double profit;

              if (p.cleanDatetime.isNull()) {
                  hold_days = (Datetime::now() - p.takeDatetime).days();
                  double cur_price = p.stock.getMarketValue(Datetime::now(), KQuery::DAY);
                  current_market_value = cur_price * p.number;
                  profit = current_market_value - (p.buyMoney - p.sellMoney + p.totalCost);
              } else {
                  hold_days = (p.cleanDatetime - p.takeDatetime).days();
                  current_market_value = 0.0;
                  profit = p.totalProfit();
              }

              hold_days_ptr[i] = hold_days;
              hold_number_ptr[i] = p.number;

              double invest = p.buyMoney - p.sellMoney + p.totalCost;
              invest_ptr[i] = invest;
              market_value_ptr[i] = current_market_value;
              profit_ptr[i] = profit;

              double profit_ratio = invest != 0.0 ? roundEx(100. * (profit / invest), 2) : 0.0;
              profit_percent_ptr[i] = profit_ratio;

              stoploss_ptr[i] = p.stoploss;
              goal_price_ptr[i] = p.goalPrice;
              clean_time_ptr[i] = p.cleanDatetime.isNull() ? std::numeric_limits<int64_t>::min()
                                                           : p.cleanDatetime.timestamp() * 1000LL;
              total_number_ptr[i] = p.totalNumber;
              total_cost_ptr[i] = p.totalCost;
              total_risk_ptr[i] = p.totalRisk;
              buy_money_ptr[i] = p.buyMoney;
              sell_money_ptr[i] = p.sellMoney;
          }

          // Build the DataFrame
          auto pandas = py::module_::import("pandas");
          py::dict columns;
          columns[htr("market_code").c_str()] =
            pandas.attr("Series")(code_list, py::arg("dtype") = "string");
          columns[htr("stock_name").c_str()] =
            pandas.attr("Series")(name_list, py::arg("dtype") = "string");
          columns[htr("take_time").c_str()] = take_time_arr.attr("astype")("datetime64[ns]");
          columns[htr("hold_days").c_str()] = hold_days_arr;
          columns[htr("hold_number").c_str()] = hold_number_arr;
          columns[htr("invest").c_str()] = invest_arr;
          columns[htr("market_value").c_str()] = market_value_arr;
          columns[htr("profit").c_str()] = profit_arr;
          columns[htr("profit_percent").c_str()] = profit_percent_arr;
          columns[htr("stoploss").c_str()] = stoploss_arr;
          columns[htr("goal_price").c_str()] = goal_price_arr;
          columns[htr("clean_time").c_str()] = clean_time_arr.attr("astype")("datetime64[ns]");
          columns[htr("total_number").c_str()] = total_number_arr;
          columns[htr("total_cost").c_str()] = total_cost_arr;
          columns[htr("total_risk").c_str()] = total_risk_arr;
          columns[htr("buy_money").c_str()] = buy_money_arr;
          columns[htr("sell_money").c_str()] = sell_money_arr;

          return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
      },
      R"(positions_to_df(positions)

    Convert the position record list to a pandas DataFrame

    Note: the calculated values such as the current market value, the profit and the profit/loss are all calculated by the daily line; when backtesting with a level below the daily line, you need to recalculate the open position records yourself!

    :param PositionRecordList positions: the position record list
    :return: a pandas DataFrame containing the position records
    :rtype: pandas.DataFrame)");
}
