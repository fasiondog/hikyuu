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
const string& (Portfolio::*pf_get_name)() const = &Portfolio::name;

void export_Portfolio() {
    class_<Portfolio>("Portfolio", init<>())
      .def(init<const string&>())
      .def(init<const TradeManagerPtr&, const SelectorPtr&, const AFPtr&>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def("get_param", &Portfolio::getParam<boost::any>)
      .def("set_param", &Portfolio::setParam<object>)
      .def("have_param", &Portfolio::haveParam)

      .add_property("name", make_function(pf_get_name, return_value_policy<copy_const_reference>()),
                    pf_set_name)
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
