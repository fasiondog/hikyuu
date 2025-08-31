/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-30
 *     Author: fasiondog
 */

#include <hikyuu/global/GlobalSpotAgent.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_SpotAgent(py::module& m) {
    m.def("start_spot_agent", startSpotAgent, py::arg("print") = false, py::arg("worker_num") = 1,
          py::arg("addr") = string(), R"(start_spot_agent([print=False, worker_num=1, addr=""])
    
    启动行情数据接收代理

    :param print: 是否打印日志
    :param worker_num: 工作线程数
    :param addr: 行情采集服务地址)");

    m.def("stop_spot_agent", stopSpotAgent, "终止行情数据接收代理");
    m.def("spot_agent_is_running", spotAgentIsRunning, "判断行情数据接收代理是否在运行");
    m.def("spot_agent_is_connected", spotAgentIsConnected, "判断行情数据接收代理是否已连接");
}
