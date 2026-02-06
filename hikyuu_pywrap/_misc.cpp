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

  并行运行多个系系统, 并返回 list FundsList, 各账户对应资产（按query时间段）

  :param sys_list: 系统列表
  :param query: 查询条件
  :param bool reset: 执行前是否依据系统部件共享属性复位
  :param bool reset_all: 强制复位所有部件)");

    m.def(
      "parallel_run_pf",
      [](const vector<PFPtr>& pf_list, const KQuery& query, bool force) {
          OStreamToPython guard(false);
          py::gil_scoped_release release;
          return parallel_run_pf(pf_list, query, force);
      },
      py::arg("pf_list"), py::arg("query"), py::arg("force") = false,
      R"(parallel_run_pf(pf_list, query[, force=False])

    并行执行多个投资组合策略, 并返回 list FundsList, 各账户对应资产（按query时间段）

    :param list pf_list: 投资组合列表
    :param Query query: 查询条件
    :param bool force: 强制重新计算)");

    m.def("get_funds_list", &getFundsList,
          R"(parallel_get_funds_list(tm_list: list, ref_dates: DatetimeList) -> list[Funds])
    
    一次性从多个账户中获取多个指定时刻的账户资金信息

    :param list tm_list: 账户列表
    :param DatetimeList ref_dates: 获取时刻列表
    :return: 账户资金列表)");

    m.def(
      "get_performance_list", &getPerformanceList, py::arg("tm_list"),
      py::arg("datetime") = Datetime::now(), py::arg("ktype") = KQuery::DAY, py::arg("ext") = true,
      R"(get_performance_list(tm_list: list, datetime: Datetime = now(), ktype: KType = DAY, ext: bool = True) -> list[Performance])
    
    一次性获取多个账户的指定时刻的账户表现

    :param list tm_list: 账户列表
    :param Datetime datetime: 指定时刻
    :param KType ktype: 指定K线类型
    :param bool ext: 是否统计扩展信息（需VIP权限，否则仍为基础统计项）
    )");
}