/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#include <atomic>
#include <functional>
#include <mutex>
#include "ShmClientHook.h"

namespace hku {
namespace ipc {

namespace {

// The forwarding callback object: it is created on the heap at the registration and published with
// release, the forwarding side reads it with acquire and then accesses it read-only; the object
// stays resident for the process lifetime and is never released, avoiding a release-use race with
// the market data thread (the forwarding reader); the registration happens only once or twice.
std::atomic<ShmClientForwarders*> g_client{nullptr};

// The interruption checker is used by both the plugin side waiting logic (runtime) and the Python
// binding (registration); it is heap allocated and never released, avoiding a static destruction
// period (such as a check triggered again in the plugin stop path) touching a destroyed TU level
// static variable
std::mutex* g_interrupt_checker_mutex = new std::mutex;
std::function<bool()>* g_interrupt_checker = new std::function<bool()>;

}  // namespace

void registerShmClient(ShmClientForwarders fwd) noexcept {
    if (!fwd) {
        // Unregister: cut the reference, the forwarding side then treats it as a safe no-op
        g_client.store(nullptr, std::memory_order_release);
        return;
    }
    // Publish the new callback group (no release any more, see above)
    g_client.store(new ShmClientForwarders(std::move(fwd)), std::memory_order_release);
}

bool shmClient() noexcept {
    return g_client.load(std::memory_order_acquire) != nullptr;
}

bool forwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                           const KRecord& record) {
    const ShmClientForwarders* fwd = g_client.load(std::memory_order_acquire);
    HKU_IF_RETURN(!fwd || !fwd->realtimeUpdate, false);
    return fwd->realtimeUpdate(market_code, ktype, record);
}

Datetime forwardGetLastUpdateTime(const std::string& market_code, const KQuery::KType& ktype) {
    const ShmClientForwarders* fwd = g_client.load(std::memory_order_acquire);
    HKU_IF_RETURN(!fwd || !fwd->getLastUpdateTime, Datetime::min());
    return fwd->getLastUpdateTime(market_code, ktype);
}

bool forwardPullFromBufferServer(const std::string& addr, const std::vector<std::string>& codes,
                                 const KQuery::KType& ktype) {
    const ShmClientForwarders* fwd = g_client.load(std::memory_order_acquire);
    HKU_IF_RETURN(!fwd || !fwd->pullFromBufferServer, false);
    return fwd->pullFromBufferServer(addr, codes, ktype);
}

void setInterruptChecker(std::function<bool()> checker) {
    std::lock_guard<std::mutex> lock(*g_interrupt_checker_mutex);
    *g_interrupt_checker = std::move(checker);
}

bool checkInterrupted() {
    std::function<bool()> checker;
    {
        std::lock_guard<std::mutex> lock(*g_interrupt_checker_mutex);
        checker = *g_interrupt_checker;
    }
    // The checker is executed outside the lock (in the Python environment it needs the GIL and
    // executing it inside the lock may deadlock)
    return checker ? checker() : false;
}

}  // namespace ipc
}  // namespace hku
