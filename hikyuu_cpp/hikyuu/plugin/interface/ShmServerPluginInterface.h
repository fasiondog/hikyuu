/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/plugin/PluginBase.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/KDataDriver.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * Data loading events: the plugin publishes the two kinds of shared memory snapshots and refreshes
 * the block cache at the correct time points according to them
 * @ingroup DataDriver
 */
enum class LoadEvent {
    BASE_DATA_READY,         ///< The base data and the ex-rights/ex-dividend data are ready (the
                             ///< historical finance has not been preloaded yet)
    BLOCKS_LOADED,           ///< The block data has been loaded
    KDATA_PRELOAD_FINISHED,  ///< The K-line preloading has been finished
    HISTORY_FINANCE_LOADED,  ///< The historical finance has been loaded
};

/**
 * The shm data service plugin interface
 * @ingroup DataDriver
 */
class HKU_API ShmServerPluginInterface : public PluginBase {
public:
    ShmServerPluginInterface() = default;
    virtual ~ShmServerPluginInterface() = default;

    /**
     * Start the shm server
     * @param datadir data directory; the current data directory of StockManager is used when it is
     *                empty
     * @param publish_shm whether to publish the two kinds of shared memory snapshots
     * @param recv_spot whether this process receives the realtime market data (it calls
     *                  startSpotAgent internally)
     * @return true is returned on a successful start; it refuses and returns false when this
     * process is already in the client mode (to prevent self-connection, see design §5.5)
     */
    virtual bool start(const std::string& datadir, bool publish_shm, bool recv_spot) noexcept = 0;

    /** Stop the shm server, unregister the mirror hook and the loading event callbacks, and release
     *  the shared memory segment */
    virtual void stop() noexcept = 0;

    /** Whether the service is running */
    virtual bool running() const noexcept = 0;

    /** The service listening address (for logs / troubleshooting; an empty string when it is not
     *  started) */
    virtual const std::string& addr() const noexcept = 0;

    // ── Client negotiation capability (the original ShmClientInterface contract is merged into
    // this interface) ───────────────────────── It is called by the StockManager client negotiation
    // path only (use_shm_server is enabled and this process is not in the server role). The
    // signatures involve the core library data types only, no plugin private type appears in the
    // interface.

    /**
     * Probe and connect to the existing shm service, blocking and waiting for its data to be ready
     * @param datadir data directory (the service address is derived from its hash, guaranteeing
     * that only the processes with the same data set interconnect)
     * @param wait_timeout_sec the total budget for waiting to be ready (seconds), 0 means waiting
     *                         infinitely
     * @return true it has been connected and the service is ready (this process should enter the
     *         client mode) | false no service / timeout / interrupted
     * @note The implementation is responsible for the connection retry and the interruption check
     *       internally (through the core library ipc::checkInterrupted()), and it must **not**
     * start the service in this process — the service can only be started explicitly by
     * start_shm_server(); after a successful connection it must register the three forwarding
     * callbacks to the core library through ipc::registerShmClient() by itself (afterwards
     * Stock::realtimeUpdate and so on are forwarded to the service process through the thin
     * forwarding layer of the core library), and unregister them on disconnection / destruction.
     */
    virtual bool connect(const std::string& datadir, uint64_t wait_timeout_sec) noexcept = 0;

    /** The connected service address (for logs) */
    virtual std::string serverAddr() const noexcept = 0;

    /** Create the K-line proxy driver (it contains a local driver connection pool internally, used
     *  for the local priority and the degradation fallback) */
    virtual KDataDriverPtr createKDataDriver(
      const KDataDriverConnectPoolPtr& local_pool) noexcept = 0;

    /** Create the basic information proxy driver (local is the local driver for the degradation
     *  fallback) */
    virtual BaseInfoDriverPtr createBaseInfoDriver(const BaseInfoDriverPtr& local) noexcept = 0;

    /** Create the block proxy driver (local is the local driver for the degradation fallback) */
    virtual BlockInfoDriverPtr createBlockDriver(const BlockInfoDriverPtr& local) noexcept = 0;
};

}  // namespace hku
