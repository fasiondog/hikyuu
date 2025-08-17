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
    py::class_<PositionRecord>(m, "PositionRecord", "持仓记录")
      .def(py::init<>())
      .def(py::init<const Stock&, const Datetime&, const Datetime&, double, price_t, price_t,
                    double, price_t, price_t, price_t, price_t>())

      .def("__str__", &PositionRecord::str)
      .def("__repr__", &PositionRecord::str)

      .def_readwrite("stock", &PositionRecord::stock, "交易对象（Stock）")
      .def_readwrite("take_datetime", &PositionRecord::takeDatetime, "初次建仓时刻（Datetime）")
      .def_readwrite("clean_datetime", &PositionRecord::cleanDatetime,
                     "平仓日期，当前持仓记录中为 constant.null_datetime")
      .def_readwrite("number", &PositionRecord::number, "当前持仓数量（float）")
      .def_readwrite("stoploss", &PositionRecord::stoploss, "当前止损价（float）")
      .def_readwrite("goal_price", &PositionRecord::goalPrice, "当前的目标价格（float）")
      .def_readwrite("total_number", &PositionRecord::totalNumber, "累计持仓数量（float）")
      .def_readwrite("buy_money", &PositionRecord::buyMoney, "累计买入资金（float）")
      .def_readwrite("total_cost", &PositionRecord::totalCost, "累计交易总成本（float）")
      .def_readwrite("total_risk", &PositionRecord::totalRisk,
                     "累计交易风险 = 各次 （买入价格-止损)*买入数量, 不包含交易成本")
      .def_readwrite("sell_money", &PositionRecord::sellMoney, "累计卖出资金（float）")
      .def_property_readonly("total_profit", &PositionRecord::totalProfit,
                             R"(total_profit(self):

    累计盈利 = 累计卖出资金 - 累计买入资金 - 累计交易成本
    注意: 只对已清仓的记录有效, 未清仓的记录返回0  )")

        DEF_PICKLE(PositionRecord);

    m.def(
      "positions_to_np",
      [](const PositionRecordList& positions) {
          size_t total = positions.size();
          HKU_IF_RETURN(total == 0, py::array());

          struct alignas(8) RawData {
              int32_t code[10];
              int32_t name[20];
              int64_t take_datetime;        // 买入日期
              int64_t hold_days;            // 已持仓天数
              double number;                // 当前持仓数量
              double invest;                // 当前投入金额
              double current_market_value;  // 当前市值
              double profit;                // 当前盈亏金额
              double profit_ratio;          // 当前盈亏比例
              double stoploss;              // 当前止损价
              double goal_price;            // 当前的目标价格
              int64_t clean_datetime;       // 卖出日期
              double total_number;          // 累计持仓数量
              double total_cost;            // 累计交易成本
              double total_risk;  // 累计交易风险 = 各次 （买入价格-止损)*买入数量, 不包含交易成本
              double buy_money;   // 累计投入金额
              double sell_money;  // 累计卖出资金
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
      R"(将持仓列表转换为Numpy
    
    注意: 其中的当前市值、利润、盈亏等计算值均以日线计算, 如使用日线一下级别回测时, 对未清仓的持仓记录需要自行重新计算！)");

    m.def(
      "positions_to_df",
      [](const PositionRecordList& positions) {
          size_t total = positions.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // 创建各列数据容器
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

          // 获取各数组缓冲区
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

          // 填充数据
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

          // 构建 DataFrame
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

    将持仓记录列表转换为 pandas DataFrame

    注意: 其中的当前市值、利润、盈亏等计算值均以日线计算, 如使用日线一下级别回测时, 对未清仓的持仓记录需要自行重新计算！

    :param PositionRecordList positions: 持仓记录列表
    :return: 包含持仓记录的 pandas DataFrame
    :rtype: pandas.DataFrame)");
}
