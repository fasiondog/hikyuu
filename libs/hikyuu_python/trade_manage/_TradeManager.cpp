/*
 * _TradeManager.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

#if HKU_PYTHON_SUPPORT_PICKLE
struct TradeManager_pickle_suite : bp::pickle_suite {
    static
    boost::python::tuple
    getinitargs(TradeManager const& w) {
        return boost::python::make_tuple(w.initDatetime(), w.initCash(),
                w.costFunc(), w.name());
    }

    static bp::object getstate(const TradeManager& param) {
        std::ostringstream os;
        OUTPUT_ARCHIVE oa(os);
        oa << param;
        return bp::str (os.str());
    }

    static void
      setstate(TradeManager& params, bp::object entries) {
        bp::str s = bp::extract<bp::str>(entries)();
        std::string st = bp::extract<std::string>(s)();
        std::istringstream is(st);

        INPUT_ARCHIVE ia(is);
        ia >> params;
      }
};
#endif /* HKU_PYTHON_SUPPORT_PICKLE */


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(cash_overload, cash, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getFundsCurve_overload, getFundsCurve, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getProfitCurve_overload, getProfitCurve, 1, 2);

FundsRecord (TradeManager::*getFunds_1)(KQuery::KType) const = &TradeManager::getFunds;
FundsRecord (TradeManager::*getFunds_2)(const Datetime&, KQuery::KType) = &TradeManager::getFunds;
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getFunds_1_overload, TradeManager::getFunds, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getFunds_2_overload, TradeManager::getFunds, 1, 2);

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(buy_overload, buy, 4, 8);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(sell_overload, sell, 3, 8);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(buyShort_overload, buyShort, 3, 8);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(sellShort_overload, sellShort, 4, 8);

TradeCostPtr (TradeManager::*get_costFunc)() const = &TradeManager::costFunc;
void (TradeManager::*set_costFunc)(const TradeCostPtr&) = &TradeManager::costFunc;

string (TradeManager::*tm_get_name)() const = &TradeManager::name;
void (TradeManager::*tm_set_name)(const string&) = &TradeManager::name;

const TradeRecordList& (TradeManager::*_getTradeList_1)() const = &TradeManager::getTradeList;
TradeRecordList (TradeManager::*_getTradeList_2)(const Datetime&, const Datetime&) const = &TradeManager::getTradeList;

void export_TradeManager() {

    size_t null_size = Null<size_t>();

    class_<TradeManager>("TradeManager", init<const Datetime&, price_t,
            const TradeCostPtr&, const string&>())
            //.def(self_ns::str(self))
            .def("__str__", &TradeManager::toString)
            .add_property("name", tm_get_name, tm_set_name)
            .add_property("initCash", &TradeManager::initCash)
            .add_property("currentCash", &TradeManager::currentCash)
            .add_property("initDatetime", &TradeManager::initDatetime)
            .add_property("firstDatetime", &TradeManager::firstDatetime)
            .add_property("lastDatetime", &TradeManager::lastDatetime)
            .add_property("reinvest", &TradeManager::reinvest)
            .add_property("precision", &TradeManager::precision)
            .add_property("costFunc", get_costFunc, set_costFunc)
            .add_property("brokeLastDatetime",
                    &TradeManager::getBrokerLastDatetime,
                    &TradeManager::setBrokerLastDatetime)

            .def("getParam", &TradeManager::getParam<boost::any>)
            .def("setParam", &TradeManager::setParam<object>)

            .def("reset", &TradeManager::reset)
            .def("clone", &TradeManager::clone)

            .def("regBroker", &TradeManager::regBroker)
            .def("clearBroker", &TradeManager::clearBroker)
            //.def("getMarginRate", &TradeManager::getMarginRate)
            .def("have", &TradeManager::have)
            .def("getStockNumber", &TradeManager::getStockNumber)
            //.def("getShortStockNumber", &TradeManager::getShortStockNumber)
            .def("getHoldNumber", &TradeManager::getHoldNumber)
            //.def("getShortHoldNumber", &TradeManager::getShortHoldNumber)

            .def("getTradeList", _getTradeList_1,
                    return_value_policy<copy_const_reference>())
            .def("getTradeList", _getTradeList_2)

            .def("getPositionList", &TradeManager::getPositionList)
            .def("getHistoryPositionList", &TradeManager::getHistoryPositionList,
                    return_value_policy<copy_const_reference>())
            .def("getPosition", &TradeManager::getPosition)
            .def("getBuyCost", &TradeManager::getBuyCost)
            .def("getSellCost", &TradeManager::getSellCost)
            //.def("getBorrowCashCost", &TradeManager::getBorrowCashCost)
            //.def("getReturnCashCost", &TradeManager::getReturnCashCost)
            //.def("getBorrowStockCost", &TradeManager::getBorrowStockCost)
            //.def("getReturnStockCost", &TradeManager::getReturnStockCost)
            .def("cash", &TradeManager::cash, cash_overload())
            .def("getFunds", getFunds_1, getFunds_1_overload())
            .def("getFunds", getFunds_2, getFunds_2_overload())
            //.def("getFunds", getFunds_1, (arg("ktype") = KQuery::DAY))
            //.def("getFunds", getFunds_2, (arg("datetime"), arg("ktype") = KQuery::DAY))
            .def("getFundsCurve", &TradeManager::getFundsCurve)
            .def("getProfitCurve", &TradeManager::getProfitCurve)

            .def("checkin", &TradeManager::checkin)
            .def("checkout", &TradeManager::checkout)
            //.def("checkinStock", &TradeManager::checkinStock)
            //.def("checkoutStock", &TradeManager::checkoutStock)
            //.def("borrowCash", &TradeManager::borrowCash)
            //.def("returnCash", &TradeManager::returnCash)
            //.def("borrowStock", &TradeManager::borrowStock)
            //.def("returnStock", &TradeManager::returnStock)

            .def("buy", &TradeManager::buy,
                    (arg("datetime"), arg("stock"),
                           arg("realPrice"), arg("num"), arg("stoploss")=0.0,
                           arg("goalPrice")=0.0, arg("planPrice")=0.0,
                           arg("part")=PART_INVALID))

                    //buy_overload(args("datetime", "stock", "realPrice",
                            //"num", "stoploss", "goalPrice", "planPrice","part")))

            .def("sell", &TradeManager::sell,
                          (arg("datetime"), arg("stock"),
                           arg("realPrice"), arg("num"), arg("stoploss")=0.0,
                           arg("goalPrice")=0.0, arg("planPrice")=0.0,
                           arg("part")=PART_INVALID))
                    //sell_overload(args("datetime", "stock", "realPrice",
                            //"num", "stoploss", "goalPrice", "planPrice", "part")))
            //.def("buyShort", &TradeManager::buyShort, buyShort_overload())
            //.def("sellShort", &TradeManager::sellShort, sellShort_overload())

            .def("addTradeRecord", &TradeManager::addTradeRecord)
            .def("tocsv", &TradeManager::tocsv)

#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(TradeManager_pickle_suite())
#endif
            ;

    register_ptr_to_python<TradeManagerPtr>();
}



