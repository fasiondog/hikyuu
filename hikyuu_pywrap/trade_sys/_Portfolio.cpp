/*
 * _Portfolio.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

BOOST_PYTHON_FUNCTION_OVERLOADS(PF_Simple_overload, PF_Simple, 0, 3);

void (Portfolio::*pf_set_name)(const string&) = &Portfolio::name;
string (Portfolio::*pf_get_name)() const= &Portfolio::name;

void export_Portfolio() {

    class_<Portfolio>("Portfolio", init<>())
            .def(init<const string&>())
            .def(init<const TradeManagerPtr&,
                      const SelectorPtr&,
                      const AFPtr&>())
            .def(self_ns::str(self))
            .def("getParam", &Portfolio::getParam<boost::any>)
            .def("setParam", &Portfolio::setParam<object>)

            .add_property("name", pf_get_name, pf_set_name)
            .add_property("tm", &Portfolio::getTM, &Portfolio::setTM)
            .add_property("se", &Portfolio::getSE, &Portfolio::setSE)
            .def("run", &Portfolio::run)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<Portfolio>())
#endif
            ;

    register_ptr_to_python<PortfolioPtr>();

    def("PF_Simple", PF_Simple, PF_Simple_overload());

}


