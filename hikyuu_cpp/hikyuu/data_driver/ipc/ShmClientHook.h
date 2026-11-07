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
 * The three realtime forwarding callbacks (registered after the shmserver plugin connects
 * successfully; an empty set unregisters them on disconnection). The forwarding implementation is
 * entirely inside the plugin; all forwardings are safe no-ops when not registered.
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
 * @note Registration happens only during the StockManager initialization negotiation; the callback
 * object lifetime follows the plugin instance and must be unregistered before it is destroyed
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
 * Register the K-line types published by the shared memory snapshot (called by the shmserver
 * plugin after a successful connect); only these types receive realtime mirror updates. An empty
 * list means the snapshot is mapped but publishes no K-line type.
 */
HKU_API void setShmCoveredKTypes(const std::vector<KQuery::KType>& ktypes) noexcept;

/** Unregister the coverage list; the coverage is unknown afterwards */
HKU_API void clearShmCoveredKTypes() noexcept;

/**
 * Register the base type of an extended K-line type; the extended type inherits its realtime
 * coverage (called by the core when the type is registered)
 */
HKU_API void registerExtraKTypeBase(const KQuery::KType& ktype,
                                    const KQuery::KType& basetype) noexcept;

/** Unregister all the extended K-line type bases (called when the extra types are released) */
HKU_API void clearExtraKTypeBases() noexcept;

/**
 * Whether the given K-line type is served by the realtime mirrored shared memory snapshot
 * @note Extended types inherit their base's coverage; with unregistered coverage the client mode
 *       conservatively returns true (callers skip the caches), the standalone mode returns false
 */
HKU_API bool isShmKTypeCovered(const KQuery::KType& ktype) noexcept;

/**
 * Set the interruption checker during a long blocking wait; true means the wait should be
 * interrupted. In Python the binding layer registers it so Ctrl+C can interrupt the wait; the
 * waiting logic stays in the plugin, which calls back ipc::checkInterrupted().
 */
HKU_API void setInterruptChecker(std::function<bool()> checker);

/** Call the registered interruption checker; false is returned when none is registered */
HKU_API bool checkInterrupted();

}  // namespace ipc
}  // namespace hku
