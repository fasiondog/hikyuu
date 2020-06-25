/*
 * main.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/hikyuu.h>

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

BOOST_PYTHON_MODULE(core) {
    boost::python::def("hikyuu_init", hku::hikyuu_init);
    boost::python::def("getStock", hku::getStock);
    boost::python::def("getVersion", hku::getVersion);

    export_DataType();
    export_Constant();
    export_util();
    export_log();
    export_Datetime();
    export_TimeDelta();
    export_MarketInfo();
    export_StockTypeInfo();
    export_StockWeight();
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

    export_io_redirect();
}
