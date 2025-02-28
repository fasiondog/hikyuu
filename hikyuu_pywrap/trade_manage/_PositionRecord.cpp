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
}
