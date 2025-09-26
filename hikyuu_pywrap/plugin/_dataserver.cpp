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
        
    启动数据缓存服务。其中save_tick 参数和 parquet_path 有关联：
    - 如果 save_tick=True, parquet_path 不为空时, 使用 parquet_path 保存数据；
    - 如果 save_tick=True, parquet_path 为空时, 则使用 clickhouse K线存储引擎保存数据(需配置使用 clickhouse K线存储引擎)

    :param str addr: 服务器地址
    :param int work_num: 工作线程数
    :param bool save_tick: 是否保存tick数据至数据库(如果 parquet_path 不为空时， 使用 parquet 文件进行保存；否则，需使用 clickhouse K线存储引擎)
    :param bool buf_tick: 是否缓存tick数据
    :param str parquet_path: 保存tick数据至parquet文件路径, 仅在 save_tick=True 时有效
    :return: None)");

    m.def("stop_data_server", stopDataServer, R"(stop_data_server()
        
    停止数据缓存服务)");

    m.def("get_data_from_buffer_server", getDataFromBufferServer,
          R"(get_data_from_buffer_server(addr: str, stklist: list, ktype: Query.KType)

    从 dataserver 数据缓存服务器拉取更新最新的缓存数据
          
    :param str addr: 数据服务器地址, 如: tcp://192.168.1.1:9201
    :param list stklist: 需要获取数据的股票列表
    :param Query.KType ktype: 数据类型)");

    m.def("get_spot_from_buffer_server", getSpotFromBufferServer,
          R"(get_spot_from_buffer_server(addr: str, market: str, code: str, datetime: str)

    从 dataserver 获取指定证券大于等于指定日期的缓存 spot 数据
          
    :param str addr: 数据服务器地址, 如: tcp://192.168.1.1:9201
    :param str market: 市场代码
    :param str code: 股票代码
    :param str datetime: 查询的日期)");
}