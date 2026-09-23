/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#pragma once

#include <functional>
#include <string>
#include <vector>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"

namespace hku {
namespace ipc {

/**
 * The three realtime forwarding callbacks (they are registered after the shmserver plugin connects
 * successfully, and an empty set is registered to unregister them on disconnection)
 * @details The implementation of the forwarding target (protocol encoding and decoding,
 * transmission, shared memory reading and writing) is entirely inside the shmserver plugin; the
 * core library does not hold any plugin interface type, it only saves the forwarding functions
 * needed by the market data path (Stock / dataserver) in this struct, and all the forwardings are
 * safe no-ops when they are not registered (standalone mode / the plugin is not installed).
 */
struct ShmClientForwarders {
    /** Stock::realtimeUpdate forwarding: true is returned when the server has applied it */
    std::function<bool(const std::string& market_code, const KQuery::KType& ktype,
                       const KRecord& record)>
      realtimeUpdate;

    /** Stock::getLastUpdateTime forwarding: it returns the last update moment of the server buffer,
     *  Datetime::min() is returned when it is not registered / it fails */
    std::function<Datetime(const std::string& market_code, const KQuery::KType& ktype)>
      getLastUpdateTime;

    /** The client asks the service process to pull the K-lines from the market data cache service
     *  (buffer server): true is returned when the server has accepted it */
    std::function<bool(const std::string& addr, const std::vector<std::string>& codes,
                       const KQuery::KType& ktype)>
      pullFromBufferServer;

    /** Whether it is empty (when all the callbacks are empty it is regarded as the unregistered
     *  state) */
    inline explicit operator bool() const noexcept {
        return static_cast<bool>(realtimeUpdate) || static_cast<bool>(getLastUpdateTime) ||
               static_cast<bool>(pullFromBufferServer);
    }
};

/**
 * Register / unregister the shm client forwarding callbacks (called when the plugin connects
 * successfully / disconnects; passing an empty set means unregistering)
 * @note The registration happens only in the StockManager initialization negotiation period, and
 * the forwarding calls happen in the market data thread; the implementation adopts lock-free
 *       publish-subscribe (publish-acquire), and the forwarding side only reads the stable object
 *       created in the registration period. The lifetime of the callback object follows the plugin
 *       instance, and it must be unregistered before the plugin is destroyed
 */
HKU_API void registerShmClient(ShmClientForwarders fwd) noexcept;

/** Whether the client forwarding is currently registered; false is returned when it is not
 *  registered (standalone mode / the plugin is not installed) */
HKU_API bool shmClient() noexcept;

/**
 * Client realtime update forwarding (called by Stock::realtimeUpdate)
 * @return true the server has applied it | false the client is not registered, the communication
 *         failed or the server did not apply it
 */
HKU_API bool forwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                   const KRecord& record);

/**
 * The client queries the service process for the last update moment of the buffer of the given
 * security x type (called by Stock::getLastUpdateTime)
 * @return the last update moment of the server buffer; Datetime::min() is returned when the client
 *         is not registered, the communication failed or the security does not exist
 */
HKU_API Datetime forwardGetLastUpdateTime(const std::string& market_code,
                                          const KQuery::KType& ktype);

/**
 * The client asks the service process to pull the latest K-line from the market data cache service
 * (buffer server) and update it
 * @return true the server has accepted it | false the client is not registered or the communication
 *         failed (the caller only logs it, without interrupting)
 */
HKU_API bool forwardPullFromBufferServer(const std::string& addr,
                                         const std::vector<std::string>& codes,
                                         const KQuery::KType& ktype);

/**
 * Set the interruption checker during a long blocking wait (such as waiting for the data service to
 * be ready); true means the wait should be interrupted
 * @details In the python environment it is registered by the binding layer (the pending signal is
 *          checked after the GIL is re-acquired), so that the wait can be interrupted by Ctrl+C. It
 *          is a general facility of the core library: the waiting logic is entirely inside the
 *          plugin, and the plugin calls back the implementation registered here through
 *          ipc::checkInterrupted().
 */
HKU_API void setInterruptChecker(std::function<bool()> checker);

/** Call the registered interruption checker; false is returned when none is registered */
HKU_API bool checkInterrupted();

}  // namespace ipc
}  // namespace hku
