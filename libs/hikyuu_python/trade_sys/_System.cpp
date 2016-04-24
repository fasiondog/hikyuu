/*
 * _System.cpp
 *
 *  Created on: 2013-3-22
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/system/build_in.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void (System::*run_monent_1)(const Datetime&) = &System::runMoment;
void (System::*run_monent_2)(const KRecord&) = &System::runMoment;

BOOST_PYTHON_FUNCTION_OVERLOADS(SYS_Simple_overload, SYS_Simple, 0, 9);

void export_System() {

    def("SYS_Simple", SYS_Simple, SYS_Simple_overload());

    def("getSystemPartName", getSystemPartName);
    def("getSystemPartEnum", getSystemPartEnum);


    class_<TradeRequest>("TradeRequest", init<>())
            //.def(self_ns::str(self))
            .def_readwrite("valid", &TradeRequest::valid)
            .def_readwrite("business", &TradeRequest::business)
            .def_readwrite("datetime", &TradeRequest::datetime)
            .def_readwrite("stoploss", &TradeRequest::stoploss)
            .def_readwrite("part", &TradeRequest::from)
            .def_readwrite("count", &TradeRequest::count)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<TradeRequest>())
#endif
            ;

    scope in_System = class_<System>("System", init<const string&>())
            .def(init<const TradeManagerPtr&,
                    const MoneyManagerPtr&,
                    const EnvironmentPtr&,
                    const ConditionPtr&,
                    const SignalPtr&,
                    const StoplossPtr&,
                    const StoplossPtr&,
                    const ProfitGoalPtr&,
                    const SlippagePtr&,
                    const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&System::name,
                            return_value_policy<copy_const_reference>()))
            .add_property("params",
                    make_function(&System::getParameter,
                            return_internal_reference<>()))
            .add_property("tm", &System::getTM, &System::setTM)
            .add_property("mm", &System::getMM, &System::setMM)
            .add_property("ev", &System::getEV, &System::setEV)
            .add_property("cn", &System::getCN, &System::setCN)
            .add_property("sg", &System::getSG, &System::setSG)
            .add_property("st", &System::getST, &System::setST)
            .add_property("tp", &System::getTP, &System::setTP)
            .add_property("pg", &System::getPG, &System::setPG)
            .add_property("sp", &System::getSP, &System::setSP)

            .def("getTradeRecordList", &System::getTradeRecordList,
                    return_value_policy<copy_const_reference>())
            .def("getBuyTradeRequest", &System::getBuyTradeRequest,
                    return_value_policy<copy_const_reference>())
            .def("getSellTradeRequest", &System::getSellTradeRequest,
                    return_value_policy<copy_const_reference>())
            .def("getSellShortTradeRequest", &System::getSellShortTradeRequest,
                    return_value_policy<copy_const_reference>())
            .def("getBuyShortTradeRequest", &System::getBuyShortTradeRequest,
                    return_value_policy<copy_const_reference>())

            .def("reset", &System::reset)
            .def("clone", &System::clone)

            .def("setTO", &System::setTO)

            .def("run", &System::run)
            .def("runMoment", run_monent_1)
            .def("runMoment", run_monent_2)
            .def("_runMoment", &System::_runMoment)

            .def("_environmentIsValid", &System::_environmentIsValid)
            .def("_conditionIsValid", &System::_conditionIsValid)
            .def("_shouldBuy", &System::_shouldBuy)
            .def("_buyNotifyAll", &System::_buyNotifyAll)
            .def("_sellNotifyAll", &System::_sellNotifyAll)
            .def("_buy", &System::_buy)
            .def("_sell", &System::_sell)
            //.def("_haveBuyRequest", &System::_haveBuyRequest)
            //.def("_haveSellRequest", &System::_haveSellRequest)
            //.def("_submitBuyRequest", &System::_submitBuyRequest)
            //.def("_submitSellRequest", &System::_submitSellRequest)
            //.def("_clearBuyRequest", &System::_clearBuyRequest)
            //.def("_clearSellRequest", &System::_clearSellRequest)

#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<System>())
#endif
            ;

    enum_<SystemPart>("SystemPart")
            .value("ENVIRONMENT", PART_ENVIRONMENT)
            .value("CONDITION", PART_CONDITION)
            .value("SIGNAL", PART_SIGNAL)
            .value("STOPLOSS", PART_STOPLOSS)
            .value("TAKEPROFIT", PART_TAKEPROFIT)
            .value("MONEYMANAGER", PART_MONEYMANAGER)
            .value("PROFITGOAL", PART_PROFITGOAL)
            .value("SLIPPAGE", PART_SLIPPAGE)
            .value("INVALID", PART_INVALID)
            ;

    register_ptr_to_python<SystemPtr>();
}
