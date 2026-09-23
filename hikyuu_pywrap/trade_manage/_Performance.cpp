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
    py::class_<Performance>(m, "Performance", "The simple performance statistics")
      .def(py::init<>())
      .def("exist", &Performance::exist)

      .def("reset", &Performance::reset, R"(reset(self)

        Reset, clearing the calculated results)")

      .def("report", &Performance::report,
           R"(report(self)

        The simple text statistics report, used to output and print directly
        It takes effect only after running statistics, or when the Performance itself is the result obtained from the TM

        :rtype: str)")

      .def("statistics", &Performance::statistics, py::arg("tm"),
           py::arg("datetime") = Datetime::now(),
           R"(statistics(self, tm[, datetime=Datetime.now()])

        According to the trade records, count the system performance up to a certain moment; datetime must be greater than or equal to lastDatetime

        :param TradeManager tm: the specified trade manager instance
        :param Datetime datetime: the statistics deadline)")

      .def("names", &Performance::names, py::return_value_policy::copy, R"(names(self)
      
      Get the names of all the statistics items)")

      .def("values", &Performance::values, R"(values(self)
      
      Get the values of all the statistics items, in the same order as names)")

      .def("to_dict",
           [](Performance& self) {
               py::dict result;
               StringList names = self.names();
               for (const auto& name : names) {
                   result[py::str(name)] = self.get(name);
               }
               return result;
           })

      .def("__getitem__", &Performance::get,
           R"(Get the indicator value by the indicator name; it takes effect only after running statistics or report
        
        :param str name: the indicator name
        :rtype: float))");
}
