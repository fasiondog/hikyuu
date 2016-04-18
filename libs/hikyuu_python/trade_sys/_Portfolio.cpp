/*
 * _Portfolio.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void (Portfolio::*set_name)(const string&) = &Portfolio::name;
string (Portfolio::*get_name)() const= &Portfolio::name;

void export_Portfolio() {
    class_<Portfolio>("Portfolio", init<>())
            .def(init<const string&>())
            .def(init<const TradeManagerPtr&,
                    const SystemPtr&,
                    const SelectorPtr&,
                    const AllocateMoneyPtr&>())
            .def(self_ns::str(self))
            .add_property("params",
                    make_function(&Portfolio::getParameter,
                            return_internal_reference<>()))
            .add_property("name", get_name, set_name)
            .add_property("am", &Portfolio::getAM, &Portfolio::setAM)
            .add_property("tm", &Portfolio::getTM, &Portfolio::setTM)
            .add_property("sys", &Portfolio::getSYS, &Portfolio::setSYS)
            .add_property("se", &Portfolio::getSE, &Portfolio::setSE)
            .def("addStock", &Portfolio::addStock)
            .def("addStockList", &Portfolio::addStockList)
            .def("run", &Portfolio::run)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<Portfolio>())
#endif
            ;

    register_ptr_to_python<PortfolioPtr>();
}


