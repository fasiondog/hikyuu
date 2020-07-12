/*
 * _PositionRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/PositionRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_PositionRecord() {
    class_<PositionRecord>("PositionRecord", "持仓记录", init<>())
      .def(init<const Stock&, const Datetime&, const Datetime&, double, price_t, price_t, double,
                price_t, price_t, price_t, price_t>())

      .def("__str__", &PositionRecord::toString)
      .def("__repr__", &PositionRecord::toString)

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
                     "累计交易风险（float） = 各次 （买入价格-止损)*买入数量, 不包含交易成本")
      .def_readwrite("sell_money", &PositionRecord::sellMoney, "累计卖出资金（float）")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<PositionRecord>())
#endif
      ;

    PositionRecordList::const_reference (PositionRecordList::*PositionRecordList_at)(
      PositionRecordList::size_type) const = &PositionRecordList::at;
    class_<PositionRecordList>("PositionRecordList",
                               "持仓记录列表，C++ std::vector<PositionRecord>包装")
      .def("__iter__", iterator<PositionRecordList>())
      .def("size", &PositionRecordList::size)
      .def("__len__", &PositionRecordList::size)
      .def("get", PositionRecordList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<PositionRecordList>())
#endif
      ;
}
