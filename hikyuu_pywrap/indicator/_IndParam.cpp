/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-07
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/indicator/Indicator.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_IndParam() {
    class_<IndParam>("IndParam", "技术指标", init<>())
      .def(init<IndicatorImpPtr>())
      .def(init<Indicator>())
      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .def("get", &IndParam::get)
      .def("get_imp", &IndParam::getImp);
}