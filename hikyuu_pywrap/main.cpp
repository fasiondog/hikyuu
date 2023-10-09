/*
 * main.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <cstdint>
#include <boost/python.hpp>
#include <hikyuu/hikyuu.h>
#include <hikyuu/global/sysinfo.h>

namespace py = boost::python;
using namespace hku;

void export_DataType();
void export_Constant();
void export_util();
void export_log();
void export_Datetime();
void export_TimeDelta();
void export_StockManager();
void export_Stock();
void export_Block();
void export_MarketInfo();
void export_StockTypeInfo();
void export_StockWeight();
void export_KQuery();
void export_KReord();
void export_TimeLineReord();
void export_TransRecord();
void export_KData();
void export_Parameter();
void export_save_load();
void export_io_redirect();

void export_data_driver_main();
void export_indicator_main();
void export_instance_main();
void export_trade_manage_main();
void export_trade_sys_main();
void export_global_main();

void export_StrategeContext();
void export_strategy_main();

KData Py_GetKData(const string& market_code, py::object start = py::long_(0),
                  py::object end = py::long_(Null<int64_t>()), KQuery::KType ktype = KQuery::DAY,
                  KQuery::RecoverType recovertType = KQuery::NO_RECOVER) {
    py::extract<KQuery> query_x(start);
    if (query_x.check()) {
        return getKData(market_code, query_x());
    }

    py::extract<int64_t> int_x(start);
    if (int_x.check()) {
        int64_t start_ix = 0, end_ix = 0;
        if (end.is_none()) {
            end_ix = Null<int64_t>();
        } else {
            py::extract<int64_t> int_y(end);
            if (!int_y.check()) {
                HKU_THROW_EXCEPTION(
                  std::invalid_argument,
                  "The input parameters start and end must be of the same type (Datetime or int)!");
            }
            end_ix = int_y();
        }
        start_ix = int_x();
        return getKData(market_code, start_ix, end_ix, ktype, recovertType);
    }

    py::extract<Datetime> date_x(start);
    if (!date_x.check()) {
        HKU_THROW_EXCEPTION(std::invalid_argument,
                            "The type of input parameter start must be Datetime or int!");
    }

    Datetime start_date, end_date;
    if (end.is_none()) {
        end_date = Null<Datetime>();
    } else {
        py::extract<Datetime> date_y(end);
        if (!date_y.check()) {
            HKU_THROW_EXCEPTION(
              std::invalid_argument,
              "The input parameters start and end must be of the same type (Datetime or int)!");
        }
        end_date = date_y();
    }

    start_date = date_x();
    return getKData(market_code, start_date, end_date, ktype, recovertType);
}

BOOST_PYTHON_MODULE(core) {
    py::docstring_options doc_options;
    doc_options.disable_signatures();

    export_DataType();
    export_Constant();
    export_util();
    export_log();
    export_Datetime();
    export_TimeDelta();
    export_MarketInfo();
    export_StockTypeInfo();
    export_StockWeight();
    export_StrategeContext();
    export_StockManager();
    export_KQuery();
    export_KReord();
    export_TimeLineReord();
    export_TransRecord();
    export_KData();
    export_Stock();
    export_Block();
    export_Parameter();
    export_save_load();

    export_data_driver_main();
    export_indicator_main();
    export_instance_main();

    export_trade_sys_main();
    export_trade_manage_main();  // must after export_trade_sys_main

    export_strategy_main();

    export_global_main();

    export_io_redirect();

    py::def("close_spend_time", close_spend_time, "全局关闭 c++ 部分耗时打印");
    py::def("open_spend_time", close_spend_time, "全局开启 c++ 部分耗时打印");

    py::def("hikyuu_init", hikyuu_init,
            (py::arg("filename"), py::arg("ignore_preload") = false,
             py::arg("context") = StrategyContext({"all"})));
    py::def("get_version", getVersion, R"(getVersion()
    
    :return: hikyuu 当前版本
    :rtype: str)");

    py::def("get_stock", getStock,
            R"(get_stock(market_code)

    根据"市场简称证券代码"获取对应的证券实例
            
    :param str market_code: 格式：“市场简称证券代码”，如"sh000001"
    :return: 对应的证券实例，如果实例不存在，则返回空实例，即Stock()，不抛出异常
    :rtype: Stock)");

    py::def(
      "get_kdata", Py_GetKData,
      (py::arg("market_code"), py::arg("start") = py::long_(0), py::arg("end") = py::object(),
       py::arg("ktype") = KQuery::DAY, py::arg("recover_type") = KQuery::NO_RECOVER),
      R"(get_kdata(market_code[, start=0, end=None, ktype=Query.DAY, recover_type=Query.NO_RECOVER])
    
    or get_kdata(market_code, query)

    获取K线数据，其中 start 和 end 需同时为 int 或 同时为 Datetime。
        
    :param str market_code: 格式：“市场简称证券代码”，如"sh000001"
    :param int or Datetime start: 起始索引或起始日期
    :param int or Datetime end: 终止索引或终止日期
    :param Query query: 查询条件
    :return: 满足查询条件的K线数据
    :rtype: KData)");
}
