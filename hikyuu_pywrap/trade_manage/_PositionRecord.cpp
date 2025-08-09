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

    m.def("positions_to_np", [](const PositionRecordList& positions) {
        struct RawData {
            char code[40];
            char name[80];
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

        std::vector<RawData> data;
        data.resize(positions.size());
        std::string ucode, uname;
        for (size_t i = 0, total = positions.size(); i < total; i++) {
            const PositionRecord& p = positions[i];
            ucode = utf8_to_utf32(p.stock.market_code(), 10);
            uname = utf8_to_utf32(p.stock.name(), 20);
            memset(data[i].code, 0, 40);
            memset(data[i].name, 0, 80);
            memcpy(data[i].code, ucode.c_str(), ucode.size() > 40 ? 40 : ucode.size());
            memcpy(data[i].name, uname.c_str(), uname.size() > 80 ? 80 : uname.size());
            data[i].take_datetime = p.takeDatetime.timestamp() / 1000LL;
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
                                       : p.cleanDatetime.timestamp() / 1000LL;
            data[i].total_number = p.totalNumber;
            data[i].total_cost = p.totalCost;
            data[i].total_risk = p.totalRisk;
            data[i].buy_money = p.buyMoney;
            data[i].sell_money = p.sellMoney;
        }

        py::dtype dtype = py::dtype(
          vector_to_python_list<string>({htr("market_code"), htr("name"), htr("take_time"),
                                         htr("hold_days"), htr("hold_number"), htr("invest"),
                                         htr("market_value"), htr("profit"), htr("profit_percent"),
                                         htr("stoploss"), htr("goal_price"), htr("clean_time"),
                                         htr("total_number"), htr("total_cost"), htr("total_risk"),
                                         htr("buy_money"), htr("sell_money")}),
          vector_to_python_list<string>({"U10", "U20", "datetime64[ms]", "i8", "d", "d", "d", "d",
                                         "d", "d", "d", "datetime64[ms]", "d", "d", "d", "d", "d"}),
          vector_to_python_list<int64_t>(
            {0, 40, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232}),
          240);

        return py::array(dtype, data.size(), data.data());
    });
}
