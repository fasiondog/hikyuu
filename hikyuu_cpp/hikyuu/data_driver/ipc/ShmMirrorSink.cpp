/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include "ShmMirrorSink.h"
#include <atomic>
#include <cstdint>

// Take the current process id, used to identify the forked child process (there is no fork on
// Windows, this is only for a unified code path)
#if defined(_WIN32)
#include <process.h>
#define HKU_SHM_GETPID _getpid
#else
#include <unistd.h>
#define HKU_SHM_GETPID getpid
#endif

namespace hku {
namespace ipc {

namespace {

/**
 * The registered mirror implementation + the pid of the registering process
 * @details The atomic fast path: when nothing is registered (a client process / shmserver not
 * started) it returns after a single acquire load. The pid gate: a forked child process on Linux
 * inherits the segment mapping and the sink pointer; if the child also triggers
 * Stock::realtimeUpdate, a second writer would appear on the same entry, breaking the single writer
 * premise of seqlock and tearing the read side; only the registering process (a matching pid)
 * forwards. The sink is registered / unregistered by the plugin at the single points start / stop.
 * The core library does no staging: the staging within the publish window, the double writing of
 * the old segment and the replay after taking over are internal responsibilities of the
 * ShmMirrorSink implementation on the plugin side (see design §5.1).
 * @note g_sink / g_sink_pid are trivially destructible namespace level atomics with no lock
 * destruction order issue at the exit
 */
std::atomic<ShmMirrorSink*> g_sink{nullptr};
std::atomic<int64_t> g_sink_pid{0};

}  // namespace

void registerShmMirrorSink(ShmMirrorSink* sink) noexcept {
    // Store the pid before the pointer: on the unregistration the pointer is set to nullptr and
    // then the pid is zeroed; register is called by the plugin from the single-threaded start/stop
    g_sink_pid.store(sink ? (int64_t)HKU_SHM_GETPID() : 0, std::memory_order_relaxed);
    g_sink.store(sink, std::memory_order_release);
}

void shmMirrorRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                             const KRecord& record) {
    ShmMirrorSink* sink = g_sink.load(std::memory_order_acquire);
    if (!sink) {
        return;
    }
    // A forked child process is not the process that registered the sink, so a write would become a
    // second writer; only the registering process reaches here and the getpid syscall cost is
    // negligible against the market data update frequency
    if (g_sink_pid.load(std::memory_order_relaxed) != (int64_t)HKU_SHM_GETPID()) {
        return;
    }
    sink->onRealtimeUpdate(market_code, ktype, record);
}

}  // namespace ipc
}  // namespace hku
