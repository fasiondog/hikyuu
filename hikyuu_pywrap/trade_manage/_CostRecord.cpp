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
    py::class_<CostRecord>(m, "CostRecord", R"(成本记录
    
    总成本 = 佣金 + 印花税 + 过户费 + 其他费用
    
    该结构主要用于存放成本记录结果，一般当做struct直接使用，
    该类本身不对总成本进行计算，也不保证上面的公式成立)")

      .def(py::init<>())
      .def(py::init<price_t, price_t, price_t, price_t, price_t>(), py::arg("commission"),
           py::arg("stamptax"), py::arg("transferfee"), py::arg("others"), py::arg("total"))

      .def("__str__", to_py_str<CostRecord>)
      .def("__repr__", to_py_str<CostRecord>)

      .def_readwrite("commission", &CostRecord::commission, "佣金")
      .def_readwrite("stamptax", &CostRecord::stamptax, "印花税")
      .def_readwrite("transferfee", &CostRecord::transferfee, "过户费")
      .def_readwrite("others", &CostRecord::others, "其他费用")
      .def_readwrite("total", &CostRecord::total,
                     "总成本(float)，= 佣金 + 印花税 + 过户费 + 其它费用")
      .def(py::self == py::self)

        DEF_PICKLE(CostRecord);
}
