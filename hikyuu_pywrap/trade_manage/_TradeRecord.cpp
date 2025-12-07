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
      .value("SUOGU", BUSINESS_SUOGU)
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
        size_t total = trades.size();
        HKU_IF_RETURN(total == 0, py::array());

        struct alignas(8) RawData {
            int32_t code[10];
            int32_t name[20];
            int64_t datetime;         // 交易日期
            int32_t business[20];     // 业务类型
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
            int32_t sig_from[20];     // 信号部件来源
            int32_t remark[100];      // 备注
        };

        RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, total = trades.size(); i < total; i++) {
            const TradeRecord& t = trades[i];
            utf8_to_utf32(t.stock.market_code(), data[i].code, 10);
            utf8_to_utf32(t.stock.name(), data[i].name, 20);
            data[i].datetime = t.datetime.timestamp() * 1000LL;
            utf8_to_utf32(getBusinessName(t.business), data[i].business, 20);
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
            utf8_to_utf32(getSystemPartName(t.from), data[i].sig_from, 20);
            utf8_to_utf32(t.remark, data[i].remark, 100);
        }

        py::dtype dtype = py::dtype(
          vector_to_python_list<string>(
            {htr("market_code"), htr("stock_name"), htr("datetime"), htr("business"),
             htr("planPrice"), htr("realPrice"), htr("goalPrice"), htr("number"), htr("stoploss"),
             htr("cash"), htr("cost_total"), htr("cost_commission"), htr("cost_stamptax"),
             htr("cost_transferfee"), htr("cost_others"), htr("part_from"), htr("remark")}),
          vector_to_python_list<string>({"U10", "U20", "datetime64[ns]", "U20", "d", "d", "d", "d",
                                         "d", "d", "d", "d", "d", "d", "d", "U20", "U100"}),
          vector_to_python_list<int64_t>(
            {0, 40, 120, 128, 208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 376}),
          776);

        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def(
      "trades_to_df",
      [](const TradeRecordList& trades) {
          size_t total = trades.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // 创建各列数据容器
          py::list code_list(total);
          py::list name_list(total);
          py::array_t<int64_t> datetime_arr(total);
          py::list business_list(total);
          py::array_t<double> planPrice_arr(total);
          py::array_t<double> realPrice_arr(total);
          py::array_t<double> goalPrice_arr(total);
          py::array_t<double> number_arr(total);
          py::array_t<double> stoploss_arr(total);
          py::array_t<double> cash_arr(total);
          py::array_t<double> cost_total_arr(total);
          py::array_t<double> cost_commission_arr(total);
          py::array_t<double> cost_stamptax_arr(total);
          py::array_t<double> cost_transferfee_arr(total);
          py::array_t<double> cost_others_arr(total);
          py::list part_from_list(total);
          py::list remark_list(total);

          // 获取各数组缓冲区
          auto datetime_buf = datetime_arr.request();
          auto planPrice_buf = planPrice_arr.request();
          auto realPrice_buf = realPrice_arr.request();
          auto goalPrice_buf = goalPrice_arr.request();
          auto number_buf = number_arr.request();
          auto stoploss_buf = stoploss_arr.request();
          auto cash_buf = cash_arr.request();
          auto cost_total_buf = cost_total_arr.request();
          auto cost_commission_buf = cost_commission_arr.request();
          auto cost_stamptax_buf = cost_stamptax_arr.request();
          auto cost_transferfee_buf = cost_transferfee_arr.request();
          auto cost_others_buf = cost_others_arr.request();

          int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
          double* planPrice_ptr = static_cast<double*>(planPrice_buf.ptr);
          double* realPrice_ptr = static_cast<double*>(realPrice_buf.ptr);
          double* goalPrice_ptr = static_cast<double*>(goalPrice_buf.ptr);
          double* number_ptr = static_cast<double*>(number_buf.ptr);
          double* stoploss_ptr = static_cast<double*>(stoploss_buf.ptr);
          double* cash_ptr = static_cast<double*>(cash_buf.ptr);
          double* cost_total_ptr = static_cast<double*>(cost_total_buf.ptr);
          double* cost_commission_ptr = static_cast<double*>(cost_commission_buf.ptr);
          double* cost_stamptax_ptr = static_cast<double*>(cost_stamptax_buf.ptr);
          double* cost_transferfee_ptr = static_cast<double*>(cost_transferfee_buf.ptr);
          double* cost_others_ptr = static_cast<double*>(cost_others_buf.ptr);

          // 填充数据
          for (size_t i = 0; i < total; i++) {
              const TradeRecord& t = trades[i];
              if (!t.stock.isNull()) {
                  code_list[i] = py::str(t.stock.market_code());
                  name_list[i] = py::str(t.stock.name());
              } else {
                  code_list[i] = py::str("");
                  name_list[i] = py::str("");
              }
              datetime_ptr[i] = t.datetime.timestamp() * 1000LL;
              business_list[i] = py::str(getBusinessName(t.business));
              planPrice_ptr[i] = t.planPrice;
              realPrice_ptr[i] = t.realPrice;
              goalPrice_ptr[i] = t.goalPrice;
              number_ptr[i] = t.number;
              stoploss_ptr[i] = t.stoploss;
              cash_ptr[i] = t.cash;
              cost_total_ptr[i] = t.cost.total;
              cost_commission_ptr[i] = t.cost.commission;
              cost_stamptax_ptr[i] = t.cost.stamptax;
              cost_transferfee_ptr[i] = t.cost.transferfee;
              cost_others_ptr[i] = t.cost.others;
              part_from_list[i] = py::str(getSystemPartName(t.from));
              remark_list[i] = py::str(t.remark);
          }

          // 构建 DataFrame
          auto pandas = py::module_::import("pandas");
          py::dict columns;
          columns[htr("market_code").c_str()] =
            pandas.attr("Series")(code_list, py::arg("dtype") = "string");
          columns[htr("stock_name").c_str()] =
            pandas.attr("Series")(name_list, py::arg("dtype") = "string");
          columns[htr("datetime").c_str()] = datetime_arr.attr("astype")("datetime64[ns]");
          columns[htr("business").c_str()] =
            pandas.attr("Series")(business_list, py::arg("dtype") = "string");
          columns[htr("planPrice").c_str()] = planPrice_arr;
          columns[htr("realPrice").c_str()] = realPrice_arr;
          columns[htr("goalPrice").c_str()] = goalPrice_arr;
          columns[htr("number").c_str()] = number_arr;
          columns[htr("stoploss").c_str()] = stoploss_arr;
          columns[htr("cash").c_str()] = cash_arr;
          columns[htr("cost_total").c_str()] = cost_total_arr;
          columns[htr("cost_commission").c_str()] = cost_commission_arr;
          columns[htr("cost_stamptax").c_str()] = cost_stamptax_arr;
          columns[htr("cost_transferfee").c_str()] = cost_transferfee_arr;
          columns[htr("cost_others").c_str()] = cost_others_arr;
          columns[htr("part_from").c_str()] =
            pandas.attr("Series")(part_from_list, py::arg("dtype") = "string");
          columns[htr("remark").c_str()] =
            pandas.attr("Series")(remark_list, py::arg("dtype") = "string");

          return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
      },
      R"(trades_to_df(trades)

    将交易记录列表转换为 pandas DataFrame

    :param TradeRecordList trades: 交易记录列表
    :return: 包含交易记录的 pandas DataFrame
    :rtype: pandas.DataFrame)");
}
