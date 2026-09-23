/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <hikyuu/plugin/dataserver.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_dataserver(py::module& m) {
    m.def(
      "start_data_server", startDataServer, py::arg("addr") = "tcp://0.0.0.0:9201",
      py::arg("work_num") = 3, py::arg("save_tick") = false, py::arg("buf_tick") = false,
      py::arg("parquet_path") = std::string(),
      R"(start_data_server(addr: str[, work_num: int=3, save_tick: bool=False, buf_tick: bool=False, parquet_path: str=''])
        
    Start the data cache server. The save_tick parameter is related to parquet_path:
    - If save_tick=True and parquet_path is not empty, use parquet_path to save the data;
    - If save_tick=True and parquet_path is empty, use the clickhouse K-line storage engine to save the data (it needs to be configured to use the clickhouse K-line storage engine)

    :param str addr: the server address
    :param int work_num: the number of the working threads
    :param bool save_tick: whether to save the tick data to the database (when parquet_path is not empty, save with the parquet files; otherwise, the clickhouse K-line storage engine needs to be used)
    :param bool buf_tick: whether to cache the tick data
    :param str parquet_path: the parquet file path to save the tick data, valid only when save_tick=True
    :return: None)");

    m.def("stop_data_server", stopDataServer, R"(stop_data_server()
        
    Stop the data cache server)");

    m.def("get_data_from_buffer_server", getDataFromBufferServer,
          R"(get_data_from_buffer_server(addr: str, stklist: list, ktype: Query.KType)

    Pull and update the latest cached data from the dataserver data cache server
          
    :param str addr: the data server address, e.g.: tcp://192.168.1.1:9201
    :param list stklist: the stock list whose data needs to be obtained
    :param Query.KType ktype: the data type)");

    m.def("get_spot_from_buffer_server", getSpotFromBufferServer,
          R"(get_spot_from_buffer_server(addr: str, market: str, code: str, datetime: str)

    Get the cached spot data of the specified security greater than or equal to the specified date from the dataserver
          
    :param str addr: the data server address, e.g.: tcp://192.168.1.1:9201
    :param str market: the market code
    :param str code: the stock code
    :param str datetime: the queried date)");
}