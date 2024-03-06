/*
 * _Performance.cpp
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/Performance.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_Performance(py::module& m) {
    py::class_<Performance>(m, "Performance", "简单绩效统计")
      .def(py::init<>())
      .def("reset", &Performance::reset, R"(reset(self)

        复位，清除已计算的结果)")

      .def("report", &Performance::report, py::arg("tm"), py::arg("datetime") = Datetime::now(),
           R"(report(self, tm[, datetime=Datetime.now()])

        简单的文本统计报告，用于直接输出打印

        :param TradeManager tm: 指定的交易管理实例
        :param Datetime datetime: 统计截止时刻
        :rtype: str)")

      .def("statistics", &Performance::statistics, py::arg("tm"),
           py::arg("datetime") = Datetime::now(),
           R"(statistics(self, tm[, datetime=Datetime.now()])

        根据交易记录，统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime

        :param TradeManager tm: 指定的交易管理实例
        :param Datetime datetime: 统计截止时刻)")

      .def("names", &Performance::names, py::return_value_policy::copy, R"(names(self)
      
      获取所有统计项名称)")

      .def("values", &Performance::values, R"(values(self)
      
      获取所有统计项值，顺序与 names 相同)")

      .def("__getitem__", &Performance::get,
           R"(按指标名称获取指标值，必须在运行 statistics 或 report 之后生效
        
        :param str name: 指标名称
        :rtype: float))");
}
