/*
 * _CostRecord.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */


#include <boost/python.hpp>
#include <hikyuu/trade_manage/CostRecord.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*eq)(const CostRecord&, const CostRecord&) = operator==;

void export_CostRecord() {
    class_<CostRecord>("CostRecord", init<>())
            .def(init<price_t, price_t, price_t, price_t, price_t>())
            .def(self_ns::str(self))
            .def_readwrite("commission", &CostRecord::commission)
            .def_readwrite("stamptax", &CostRecord::stamptax)
            .def_readwrite("transferfee", &CostRecord::transferfee)
            .def_readwrite("others", &CostRecord::others)
            .def_readwrite("total", &CostRecord::total)
            .def("__eq__", eq)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<CostRecord>())
#endif
            ;
}
