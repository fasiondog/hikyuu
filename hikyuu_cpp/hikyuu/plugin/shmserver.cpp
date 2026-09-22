/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include <atomic>
#include "hikyuu/StockManager.h"
#include "interface/plugins.h"
#include "shmserver.h"

namespace hku {

namespace {
// The facade holds the service plugin pointer of "this process has started": stop / isRunning only
// query this pointer and never go through the on-demand loading of getPlugin. Reason:
// stopShmServer() is called unconditionally by GlobalInitializer::clean() at every process exit;
// going through getPlugin would repeatedly try dlopen on an ordinary standalone process without an
// installed / started shmserver and report by mistake, polluting the exit path.
std::atomic<ShmServerPluginInterface*> g_shm_server_plugin{nullptr};
}  // namespace

bool HKU_API startShmServer(const std::string& datadir, bool publish_shm, bool recv_spot) noexcept {
    // The server role is set first: even when StockManager has not been initialized,
    // _negotiateShmServer() skips the client negotiation accordingly, preventing the service
    // process from wrongly entering the client mode and forming a realtimeUpdate self-forwarding
    // loop (see design §5.5)
    setShmServerRole(true);

    // Idempotent: a success is returned directly when the service has been started in this process
    HKU_INFO_IF_RETURN(g_shm_server_plugin.load(std::memory_order_acquire) != nullptr, true,
                       "hikyuu shm server is already running.");

    StockManager& sm = StockManager::instance();
    ShmServerPluginInterface* plugin =
      sm.getPlugin<ShmServerPluginInterface>(HKU_PLUGIN_SHM_SERVER);
    HKU_ERROR_IF_RETURN(!plugin, false,
                        "Can't find {} plugin! It is a VIP plugin; check that it is installed and "
                        "properly licensed.",
                        HKU_PLUGIN_SHM_SERVER);

    // The plugin start() is responsible internally for: the authorization check, the
    // self-connection guard (rejected when isIpcClientMode is true), registering the ShmMirrorSink
    // and the LoadEvent callbacks, startSpotAgent (recv_spot) and the first publish
    HKU_ERROR_IF_RETURN(!plugin->start(datadir, publish_shm, recv_spot), false,
                        "Failed to start hikyuu shm server plugin!");

    g_shm_server_plugin.store(plugin, std::memory_order_release);
    return true;
}

void HKU_API stopShmServer() noexcept {
    // Take and clear the pointer: the exchange guarantees the idempotence of stop and isRunning
    // immediately reports not running afterwards
    ShmServerPluginInterface* plugin =
      g_shm_server_plugin.exchange(nullptr, std::memory_order_acq_rel);
    HKU_IF_RETURN(!plugin, void());
    // The plugin stop() does internally, in order: stopping the market data, unregistering the
    // ShmMirrorSink, unregistering the LoadEvent callbacks, ShmServer::stop(true) and destroying
    // the publisher
    plugin->stop();
}

bool HKU_API isShmServerRunning() noexcept {
    ShmServerPluginInterface* plugin = g_shm_server_plugin.load(std::memory_order_acquire);
    return plugin ? plugin->running() : false;
}

}  // namespace hku
