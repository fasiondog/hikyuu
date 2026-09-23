/*
 * _CostRecord.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/CostRecord.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_CostRecord(py::module& m) {
    py::class_<CostRecord>(m, "CostRecord", R"(The cost record
    
    The total cost = the commission + the stamp tax + the transfer fee + the other fees
    
    This structure is mainly used to store the cost record results, generally used directly as a struct,
    and the class itself does not calculate the total cost, nor guarantee that the above formula holds)")

      .def(py::init<>())
      .def(py::init<price_t, price_t, price_t, price_t, price_t>(), py::arg("commission"),
           py::arg("stamptax"), py::arg("transferfee"), py::arg("others"), py::arg("total"))

      .def("__str__", to_py_str<CostRecord>)
      .def("__repr__", to_py_str<CostRecord>)

      .def_readwrite("commission", &CostRecord::commission, "The commission")
      .def_readwrite("stamptax", &CostRecord::stamptax, "The stamp tax")
      .def_readwrite("transferfee", &CostRecord::transferfee, "The transfer fee")
      .def_readwrite("others", &CostRecord::others, "The other fees")
      .def_readwrite("total", &CostRecord::total,
                     "The total cost (float), = the commission + the stamp tax + the transfer fee + the other fees")
      .def(py::self == py::self)

        DEF_PICKLE(CostRecord);
}
