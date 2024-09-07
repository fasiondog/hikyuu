/*
 * main.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <cstdint>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <hikyuu/hikyuu.h>
#include <hikyuu/global/sysinfo.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_bind_stl(py::module& m);
void export_DataType(py::module& m);
void export_Constant(py::module& m);
void export_util(py::module& m);
void export_log(py::module& m);
void export_Datetime(py::module& m);
void export_TimeDelta(py::module& m);
void export_StockManager(py::module& m);
void export_Stock(py::module& m);
void export_Block(py::module& m);
void export_MarketInfo(py::module& m);
void export_StockTypeInfo(py::module& m);
void export_StockWeight(py::module& m);
void export_KQuery(py::module& m);
void export_KReord(py::module& m);
void export_TimeLineReord(py::module& m);
void export_TransRecord(py::module& m);
void export_KData(py::module& m);
void export_Parameter(py::module& m);
void export_io_redirect(py::module& m);

void export_data_driver_main(py::module& m);
void export_indicator_main(py::module& m);
void export_SystemPart(py::module& m);
void export_trade_manage_main(py::module& m);
void export_trade_sys_main(py::module& m);
void export_global_main(py::module& m);
void export_analysis_main(py::module& m);

void export_StrategeContext(py::module& m);
void export_strategy_main(py::module& m);

#if PY_MINOR_VERSION == 8
PYBIND11_MODULE(core38, m) {
#elif PY_MINOR_VERSION == 9
PYBIND11_MODULE(core39, m) {
#elif PY_MINOR_VERSION == 10
PYBIND11_MODULE(core310, m) {
#elif PY_MINOR_VERSION == 11
PYBIND11_MODULE(core311, m) {
#elif PY_MINOR_VERSION == 12
PYBIND11_MODULE(core312, m) {
#else
PYBIND11_MODULE(core, m) {
#endif

    py::register_exception<hku::exception>(m, "HKUException");

    // 设置系统运行状态
    setRunningInPython(true);

#if HKU_ENABLE_SEND_FEEDBACK
    sendPythonVersionFeedBack(PY_MAJOR_VERSION, PY_MINOR_VERSION, PY_MICRO_VERSION);
#endif

    export_bind_stl(m);
    export_DataType(m);
    export_Constant(m);
    export_util(m);
    export_log(m);
    export_Datetime(m);
    export_TimeDelta(m);

    export_MarketInfo(m);
    export_StockTypeInfo(m);
    export_StockWeight(m);
    export_StrategeContext(m);
    export_StockManager(m);
    export_KQuery(m);
    export_KReord(m);
    export_TimeLineReord(m);
    export_TransRecord(m);
    export_KData(m);
    export_Stock(m);
    export_Block(m);
    export_Parameter(m);

    export_data_driver_main(m);
    export_indicator_main(m);

    export_SystemPart(m);
    export_trade_manage_main(m);
    export_trade_sys_main(m);  // must after export_trade_manage_main

    export_analysis_main(m);
    export_strategy_main(m);

    export_global_main(m);
    export_io_redirect(m);

    m.def("set_python_in_jupyter", setPythonInJupyter);
    m.def("set_python_in_interactive", setPythonInInteractive);

    m.def("close_spend_time", close_spend_time, "全局关闭 c++ 部分耗时打印");
    m.def("open_spend_time", close_spend_time, "全局开启 c++ 部分耗时打印");

    m.def("hikyuu_init", hikyuu_init, py::arg("filename"), py::arg("ignore_preload") = false,
          py::arg("context") = StrategyContext({"all"}));
    m.def("get_version", getVersion, R"(getVersion()

        :return: hikyuu 当前版本
        :rtype: str)");

    m.def("get_version_with_build", getVersionWithBuild);
    m.def("get_version_git", getVersionWithGit);
    m.def("get_last_version", getLatestVersion);
    m.def("can_upgrade", CanUpgrade);

    m.def("get_stock", getStock,
          R"(get_stock(market_code)

        根据"市场简称证券代码"获取对应的证券实例

        :param str market_code: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则返回空实例，即Stock()，不抛出异常
        :rtype: Stock)");

    int64_t null_int64 = Null<int64_t>();
    Datetime null_date = Null<Datetime>();

    m.def("get_block", getBlock, R"(get_block(category: str, name: str)
    
    获取预定义板块

    :param str category: 板块分类
    :param str name: 板块名称
    :rtype: Block)");

    m.def("get_kdata", py::overload_cast<const string&, const KQuery&>(getKData));

    m.def(
      "get_kdata",
      py::overload_cast<const string&, int64_t, int64_t, const KQuery::KType&, KQuery::RecoverType>(
        getKData),
      py::arg("market_code"), py::arg("start") = 0, py::arg("end") = null_int64,
      py::arg("ktype") = KQuery::DAY, py::arg("recover_type") = KQuery::NO_RECOVER,
      R"(根据证券代码及起止位置获取 [start, end) 范围的 K 线数据

    :param str market_code: 证券代码，如: 'sh000001'
    :param int start: 起始索引
    :param int end: 结束索引
    :param Query.KType ktype: K 线类型, 'DAY'|'WEEK'|'MONTH'|'QUARTER'|'HALFYEAR'|'YEAR'|'MIN'|'MIN5'|'MIN15'|'MIN30'|'MIN60'
    :param Query.RecoverType recover_type: 复权类型)");

    m.def("get_kdata",
          py::overload_cast<const string&, const Datetime&, const Datetime&, const KQuery::KType&,
                            KQuery::RecoverType>(getKData),
          py::arg("market_code"), py::arg("start") = Datetime::min(), py::arg("end") = null_date,
          py::arg("ktype") = KQuery::DAY, py::arg("recover_type") = KQuery::NO_RECOVER,
          R"(根据证券代码及起止日期获取 [start, end) 范围的 K 线数据

    :param str market_code: 证券代码，如: 'sh000001'
    :param int start: 起始日期
    :param int end: 结束日期
    :param Query.KType ktype: K 线类型, 'DAY'|'WEEK'|'MONTH'|'QUARTER'|'HALFYEAR'|'YEAR'|'MIN'|'MIN5'|'MIN15'|'MIN30'|'MIN60'
    :param Query.RecoverType recover_type: 复权类型)");
}
