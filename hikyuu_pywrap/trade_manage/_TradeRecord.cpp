/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/TradeRecord.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TradeRecord() {
    enum_<BUSINESS>("BUSINESS")
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

    def("get_business_name", getBusinessName, R"(get_business_name(business)

    :param BUSINESS business: 交易业务类型
    :return: 交易业务类型名称("INIT"|"BUY"|"SELL"|"GIFT"|"BONUS"|"CHECKIN"|"CHECKOUT"|"UNKNOWN"
    :rtype: string)");

    class_<TradeRecord>("TradeRecord", "交易记录", init<>())
      .def(init<const Stock&, const Datetime&, BUSINESS, price_t, price_t, price_t, double,
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

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TradeRecord>())
#endif
      ;

    class_<TradeRecordList>("TradeRecordList")
      .def(vector_indexing_suite<TradeRecordList>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TradeRecordList>())
#endif
      ;
}
