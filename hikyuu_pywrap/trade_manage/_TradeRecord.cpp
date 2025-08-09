/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/TradeRecord.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TradeRecord(py::module& m) {
    py::enum_<BUSINESS>(m, "BUSINESS")
      .value("INIT", BUSINESS_INIT)
      .value("BUY", BUSINESS_BUY)
      .value("SELL", BUSINESS_SELL)
      .value("BUY_SHORT", BUSINESS_BUY_SHORT)
      .value("SELL_SHORT", BUSINESS_SELL_SHORT)
      .value("GIFT", BUSINESS_GIFT)
      .value("BONUS", BUSINESS_BONUS)
      .value("CHECKIN", BUSINESS_CHECKIN)
      .value("CHECKOUT", BUSINESS_CHECKOUT)
      .value("CHECKIN_STOCK", BUSINESS_CHECKIN_STOCK)
      .value("CHECKOUT_STOCK", BUSINESS_CHECKOUT_STOCK)
      .value("BORROW_CASH", BUSINESS_BORROW_CASH)
      .value("RETURN_CASH", BUSINESS_RETURN_CASH)
      .value("BORROW_STOCK", BUSINESS_BORROW_STOCK)
      .value("RETURN_STOCK", BUSINESS_RETURN_STOCK)
      .value("INVALID", BUSINESS_INVALID);

    m.def("get_business_name", getBusinessName, R"(get_business_name(business)

    :param BUSINESS business: 交易业务类型
    :return: 交易业务类型名称("INIT"|"BUY"|"SELL"|"GIFT"|"BONUS"|"CHECKIN"|"CHECKOUT"|"UNKNOWN"
    :rtype: string)");

    py::class_<TradeRecord>(m, "TradeRecord", "交易记录")
      .def(py::init<>())
      .def(py::init<const Stock&, const Datetime&, BUSINESS, price_t, price_t, price_t, double,
                    const CostRecord&, price_t, price_t, SystemPart>())

      .def("__str__", &TradeRecord::toString)
      .def("__repr__", &TradeRecord::toString)

      .def("is_null", &TradeRecord::isNull)

      .def_readwrite("stock", &TradeRecord::stock, "股票（Stock）")
      .def_readwrite("datetime", &TradeRecord::datetime, " 交易时间（Datetime）")
      .def_readwrite("business", &TradeRecord::business, "交易类型（BUSINESS）")
      .def_readwrite("plan_price", &TradeRecord::planPrice, "计划交易价格（float）")
      .def_readwrite("real_price", &TradeRecord::realPrice, "实际交易价格（float）")
      .def_readwrite("goal_price", &TradeRecord::goalPrice,
                     "目标价格（float），如果为0表示未限定目标")
      .def_readwrite("number", &TradeRecord::number, "成交数量（float）")
      .def_readwrite("cost", &TradeRecord::cost, "交易成本")
      .def_readwrite("stoploss", &TradeRecord::stoploss, "止损价（float）")
      .def_readwrite("cash", &TradeRecord::cash, "现金余额（float）")
      .def_readwrite("part", &TradeRecord::from,
                     "交易指示来源，区别是交易系统哪个部件发出的指示，参见： "
                     ":py:class:`System.Part`")  // python中不能用from关键字
      .def_readwrite("remark", &TradeRecord::remark, "备注")

        DEF_PICKLE(TradeRecord);

    m.def("trades_to_np", [](const TradeRecordList& trades) {
        struct RawData {
            char code[40];
            char name[80];
            int64_t datetime;         // 交易日期
            char business[80];        // 业务类型
            double planPrice;         // 计划交易价格
            double realPrice;         // 实际交易价格
            double goalPrice;         // 目标价位，如果为0或Null表示未限定目标
            double number;            // 成交数量
            double stoploss;          // 止损价
            double cash;              // 现金余额
            double cost_total;        // 总成本
            double cost_commission;   // 佣金
            double cost_stamptax;     // 印花税
            double cost_transferfee;  // 过户费
            double cost_others;       // 其他费用
            char sig_from[80];        // 信号部件来源
            char remark[400];         // 备注
        };

        std::vector<RawData> data;
        data.resize(trades.size());
        for (size_t i = 0, total = trades.size(); i < total; i++) {
            const TradeRecord& t = trades[i];
            memset(data[i].code, 0, 40);
            memset(data[i].name, 0, 80);
            if (!t.stock.isNull()) {
                auto ucode = utf8_to_utf32(t.stock.market_code(), 10);
                auto uname = utf8_to_utf32(t.stock.name(), 20);
                memcpy(data[i].code, ucode.c_str(), ucode.size() > 40 ? 40 : ucode.size());
                memcpy(data[i].name, uname.c_str(), uname.size() > 80 ? 80 : uname.size());
            }
            data[i].datetime = t.datetime.timestamp() / 1000LL;
            auto business = utf8_to_utf32(getBusinessName(t.business), 20);
            memset(data[i].business, 0, 80);
            memcpy(data[i].business, business.c_str(), business.size() > 80 ? 80 : business.size());
            data[i].planPrice = t.planPrice;
            data[i].realPrice = t.realPrice;
            data[i].goalPrice = t.goalPrice;
            data[i].number = t.number;
            data[i].stoploss = t.stoploss;
            data[i].cash = t.cash;
            data[i].cost_total = t.cost.total;
            data[i].cost_commission = t.cost.commission;
            data[i].cost_stamptax = t.cost.stamptax;
            data[i].cost_transferfee = t.cost.transferfee;
            data[i].cost_others = t.cost.others;
            auto sig = utf8_to_utf32(getSystemPartName(t.from), 20);
            memset(data[i].sig_from, 0, 80);
            memcpy(data[i].sig_from, sig.c_str(), sig.size() > 80 ? 80 : sig.size());
            auto remark = utf8_to_utf32(t.remark, 100);
            memset(data[i].remark, 0, 400);
            memcpy(data[i].remark, remark.c_str(), remark.size() > 400 ? 400 : remark.size());
        }

        py::dtype dtype = py::dtype(
          vector_to_python_list<string>(
            {htr("market_code"), htr("name"), htr("datetime"), htr("business"), htr("planPrice"),
             htr("realPrice"), htr("goalPrice"), htr("number"), htr("stoploss"), htr("cash"),
             htr("cost_total"), htr("cost_commission"), htr("cost_stamptax"),
             htr("cost_transferfee"), htr("cost_others"), htr("part_from"), htr("remark")}),
          vector_to_python_list<string>({"U10", "U20", "datetime64[ms]", "U20", "d", "d", "d", "d",
                                         "d", "d", "d", "d", "d", "d", "d", "U20", "U100"}),
          vector_to_python_list<int64_t>(
            {0, 40, 120, 128, 208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 376}),
          776);

        return py::array(dtype, data.size(), data.data());
    });
}
