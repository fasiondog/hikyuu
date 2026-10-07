/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-17
 *      Author: fasiondog
 */

#include <hikyuu/misc.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_misc(py::module& m) {
    m.def("parallel_run_sys", &parallel_run_sys, py::arg("sys_list"), py::arg("query"),
          py::arg("reset") = false, py::arg("reset_all") = false,
          R"(parallel_run_sys(sys_list, query[, reset=False, reset_all=False])

  Run multiple systems in parallel, and return a list of FundsList, the assets of each account (within the query time range)

  :param sys_list: the system list
  :param query: the query condition
  :param bool reset: whether to reset according to the sharing attributes of the system parts before executing
  :param bool reset_all: forcibly reset all the parts)");




    m.def("get_funds_list", &getFundsList,
          R"(parallel_get_funds_list(tm_list: list, ref_dates: DatetimeList) -> list[Funds])
    
    Get the account fund information of multiple specified moments from multiple accounts at once

    :param list tm_list: the account list
    :param DatetimeList ref_dates: the list of the moments to get
    :return: the list of the account funds)");

    m.def(
      "get_performance_list", &getPerformanceList, py::arg("tm_list"),
      py::arg("datetime") = Datetime::now(), py::arg("ktype") = KQuery::DAY, py::arg("ext") = true,
      R"(get_performance_list(tm_list: list, datetime: Datetime = now(), ktype: KType = DAY, ext: bool = True) -> list[Performance])
    
    Get the account performance of multiple accounts at the specified moment at once

    :param list tm_list: the account list
    :param Datetime datetime: the specified moment
    :param KType ktype: the specified K-line type
    :param bool ext: whether to count the extended information (requiring the donating user permission; otherwise, they are still the basic statistics items)
    )");
}