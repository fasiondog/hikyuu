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

    Start the shm (shared memory) data server within the current process, for the other hikyuu processes to read with zero copy as the clients, avoiding the repeated data loading of the multiple processes.
    The service is provided by the standalone shmserver plugin (a VIP plugin, requiring a valid license).
    The client processes need to explicitly enable use_shm_server (in the configuration file or load_hikyuu(use_shm_server=True)) to join this service.

    It must be called after the hikyuu initialization (import hikyuu completes the initialization by default); calling it before the initialization will return False because the data is not ready.

    :param str datadir: the data directory; when it is empty, the current StockManager data directory is used
    :param bool publish_shm: whether to publish the two kinds of the shared memory snapshots (the K-line hot data + the basic information)
    :param bool recv_spot: whether this process receives the real-time market data (internally subscribing to quotation_server and driving the real-time updates)
    :return: return True when started successfully; return False when this process is already in the client mode, the plugin is missing or the license is invalid)");

    m.def("stop_shm_server", stopShmServer, R"(stop_shm_server() -> None

    Stop the shm data server within the current process, releasing the shared memory segments and unregistering the related hooks)");

    m.def("is_shm_server_running", isShmServerRunning, R"(is_shm_server_running() -> bool

    Query whether the shm data server within the current process is running

    :return: return True when running, otherwise False)");
}
