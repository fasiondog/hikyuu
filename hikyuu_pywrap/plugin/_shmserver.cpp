/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include <hikyuu/plugin/shmserver.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

void export_plugin_shmserver(py::module& m) {
    m.def(
      "start_shm_server", startShmServer, py::arg("datadir") = std::string(),
      py::arg("publish_shm") = true, py::arg("recv_spot") = true,
      R"(start_shm_server(datadir: str='', publish_shm: bool=True, recv_spot: bool=True) -> bool

    在当前进程内启动 shm(共享内存)数据服务，供其他 hikyuu 进程作为客户端零拷贝读取，避免多进程重复加载数据。
    服务由独立插件 shmserver 提供（VIP 插件，需有效授权）。
    客户端进程需显式开启 use_shm_server（配置文件或 load_hikyuu(use_shm_server=True)）才会接入本服务。

    须在 hikyuu 初始化之后调用（import hikyuu 默认完成初始化）；早于初始化调用会因数据未就绪而返回 False。

    :param str datadir: 数据目录，为空时使用当前 StockManager 数据目录
    :param bool publish_shm: 是否发布两类共享内存快照（K线热数据 + 基础信息）
    :param bool recv_spot: 是否由本进程接收实时行情（内部订阅 quotation_server 并驱动实时更新）
    :return: 启动成功返回 True；本进程已处于客户端模式、插件缺失或授权无效时返回 False)");

    m.def("stop_shm_server", stopShmServer, R"(stop_shm_server() -> None

    停止当前进程内的 shm 数据服务，释放共享内存段并注销相关挂钩)");

    m.def("is_shm_server_running", isShmServerRunning, R"(is_shm_server_running() -> bool

    查询当前进程内的 shm 数据服务是否在运行

    :return: 运行中返回 True，否则 False)");
}
