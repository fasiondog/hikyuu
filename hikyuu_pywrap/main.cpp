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

void export_DataType(py::module& m);
void export_Constant(py::module& m);
void export_util(py::module& m);
// void export_analysis();
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
// void export_save_load();
void export_io_redirect(py::module& m);

void export_data_driver_main(py::module& m);
// void export_indicator_main();
// void export_instance_main();
void export_trade_manage_main(py::module& m);
void export_trade_sys_main(py::module& m);
// void export_global_main();

void export_StrategeContext(py::module& m);
// void export_strategy_main();

// KData Py_GetKData(const string& market_code, py::object start = py::long_(0),
//                   py::object end = py::long_(Null<int64_t>()), KQuery::KType ktype = KQuery::DAY,
//                   KQuery::RecoverType recovertType = KQuery::NO_RECOVER) {
//     py::extract<KQuery> query_x(start);
//     if (query_x.check()) {
//         return getKData(market_code, query_x());
//     }

//     py::extract<int64_t> int_x(start);
//     if (int_x.check()) {
//         int64_t start_ix = 0, end_ix = 0;
//         if (end.is_none()) {
//             end_ix = Null<int64_t>();
//         } else {
//             py::extract<int64_t> int_y(end);
//             if (!int_y.check()) {
//                 HKU_THROW_EXCEPTION(
//                   std::invalid_argument,
//                   "The input parameters start and end must be of the same type (Datetime or
//                   int)!");
//             }
//             end_ix = int_y();
//         }
//         start_ix = int_x();
//         return getKData(market_code, start_ix, end_ix, ktype, recovertType);
//     }

//     py::extract<Datetime> date_x(start);
//     if (!date_x.check()) {
//         HKU_THROW_EXCEPTION(std::invalid_argument,
//                             "The type of input parameter start must be Datetime or int!");
//     }

//     Datetime start_date, end_date;
//     if (end.is_none()) {
//         end_date = Null<Datetime>();
//     } else {
//         py::extract<Datetime> date_y(end);
//         if (!date_y.check()) {
//             HKU_THROW_EXCEPTION(
//               std::invalid_argument,
//               "The input parameters start and end must be of the same type (Datetime or int)!");
//         }
//         end_date = date_y();
//     }

//     start_date = date_x();
//     return getKData(market_code, start_date, end_date, ktype, recovertType);
// }

// BOOST_PYTHON_MODULE(core) {
//     py::docstring_options doc_options;
//     doc_options.disable_signatures();
PYBIND11_MODULE(core, m) {
    export_DataType(m);
    export_Constant(m);
    export_util(m);
    //     export_analysis();
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
    //     export_save_load();

    export_data_driver_main(m);
    //     export_indicator_main();
    //     export_instance_main();

    export_trade_sys_main(m);
    export_trade_manage_main(m);  // must after export_trade_sys_main

    //     export_strategy_main();

    //     export_global_main();

    export_io_redirect(m);

    //     py::def("close_spend_time", close_spend_time, "全局关闭 c++ 部分耗时打印");
    //     py::def("open_spend_time", close_spend_time, "全局开启 c++ 部分耗时打印");

    m.def("hikyuu_init", hikyuu_init, py::arg("filename"), py::arg("ignore_preload") = false,
          py::arg("context") = StrategyContext({"all"}));
    m.def("get_version", getVersion, R"(getVersion()

        :return: hikyuu 当前版本
        :rtype: str)");

    m.def("get_version_with_build", getVersionWithBuild);

    m.def("get_stock", getStock,
          R"(get_stock(market_code)

        根据"市场简称证券代码"获取对应的证券实例

        :param str market_code: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则返回空实例，即Stock()，不抛出异常
        :rtype: Stock)");

    int64_t null_int64 = Null<int64_t>();
    Datetime null_date = Null<Datetime>();

    m.def("get_kdata",
          py::overload_cast<const string&, int64_t, int64_t, KQuery::KType, KQuery::RecoverType>(
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
          py::overload_cast<const string&, const Datetime&, const Datetime&, KQuery::KType,
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
