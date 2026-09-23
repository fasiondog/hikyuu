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
    
    Start the market data receiving agent

    :param print: whether to print the logs
    :param worker_num: the number of the working threads
    :param addr: the market data collection service address)");

    m.def("stop_spot_agent", stopSpotAgent, "Stop the market data receiving agent");
    m.def("spot_agent_is_running", spotAgentIsRunning, "Judge whether the market data receiving agent is running");
    m.def("spot_agent_is_connected", spotAgentIsConnected, "Judge whether the market data receiving agent is connected");
}
