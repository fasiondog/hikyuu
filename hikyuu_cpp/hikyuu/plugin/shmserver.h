/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/DataType.h"  // Provides the empty fallback definition of HKU_API (mac/linux does
                              // not inject -D HKU_API)

namespace hku {

/**
 * Start the shm data service in the current process (it loads the shmserver plugin and starts the
 * server side)
 * @details The paradigm is the same as startDataServer: the facade first sets the server role (to
 *          prevent self-connection, see design §5.5), then loads the shmserver plugin through
 *          StockManager and calls its start(). The plugin requires a VIP license, false is returned
 *          when it is not licensed / not installed.
 * @param datadir data directory; the current data directory of StockManager is used when it is
 * empty
 * @param publish_shm whether to publish the two kinds of shared memory snapshots (the K-line hot
 *                    data + the basic information)
 * @param recv_spot whether this process receives the realtime market data (it calls startSpotAgent
 *                  internally, and it must be after init)
 * @return true is returned on a successful start; false is returned when this process is already in
 *         the client mode, the plugin is missing or the license is invalid
 * @note It must be called after the main program initialization (which is completed by importing
 *       hikyuu by default); calling it earlier than init fails because the data is not ready
 * @ingroup DataDriver
 */
bool HKU_API startShmServer(const std::string& datadir = "", bool publish_shm = true,
                            bool recv_spot = true) noexcept;

/**
 * Stop the shm data service in the current process
 * @details It must be earlier than nng_fini(): the server side nng worker holds the in-flight
 *          receiving operations, and if it is left until after the global nng state is torn down
 *          and then triggered by the cancel callback, it would reload the receiving on the
 * destroyed internal structures and crash. It is called by GlobalInitializer::clean().
 * @ingroup DataDriver
 */
void HKU_API stopShmServer() noexcept;

/**
 * Query whether the shm data service in the current process is running
 * @return true is returned when the service is running; false is returned when the plugin is not
 *         loaded / not started
 * @ingroup DataDriver
 */
bool HKU_API isShmServerRunning() noexcept;

}  // namespace hku
