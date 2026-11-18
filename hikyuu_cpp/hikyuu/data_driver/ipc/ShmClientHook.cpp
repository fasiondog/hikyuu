/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#include <algorithm>
#include <atomic>
#include <cctype>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>
#include "hikyuu/StockManager.h"
#include "hikyuu/utilities/arithmetic.h"
#include "ShmClientHook.h"

namespace hku {
namespace ipc {

// Heap-allocated forwarding callbacks published with release / read with acquire; they stay
// resident for the process lifetime, avoiding a release-use race with the forwarding reader
static std::atomic<ShmClientForwarders*> g_client{nullptr};

struct CoveredKTypes {
    std::vector<std::string> ktypes;
};

// Immutable snapshot published once per connection, kept resident for the same reason as g_client
static std::atomic<const CoveredKTypes*> g_covered_ktypes{nullptr};

struct ExtraKTypeBases {
    std::vector<std::pair<std::string, std::string>> bases;
};

// Immutable snapshot replaced at registration, resident like g_client; single-threaded init only
static std::atomic<const ExtraKTypeBases*> g_extra_kbase{nullptr};

// Same conversion rule as hku::to_upper, applied per character to avoid allocating on every query
static char upperChar(char c) noexcept {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

static bool equalsIgnoreCase(std::string_view lhs, std::string_view rhs) noexcept {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (upperChar(lhs[i]) != upperChar(rhs[i])) {
            return false;
        }
    }
    return true;
}

// The interruption checker is used by both the plugin side waiting logic (runtime) and the Python
// binding (registration); it is heap allocated and never released, avoiding a static destruction
// period (such as a check triggered again in the plugin stop path) touching a destroyed TU level
// static variable
static std::mutex* g_interrupt_checker_mutex = new std::mutex;
static std::function<bool()>* g_interrupt_checker = new std::function<bool()>;

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

void setShmCoveredKTypes(const std::vector<KQuery::KType>& ktypes) noexcept {
    try {
        auto covered = std::make_unique<CoveredKTypes>();
        covered->ktypes.reserve(ktypes.size());
        for (const auto& ktype : ktypes) {
            std::string upper(ktype);
            to_upper(upper);
            if (std::find(covered->ktypes.begin(), covered->ktypes.end(), upper) ==
                covered->ktypes.end()) {
                covered->ktypes.push_back(upper);
            }
        }
        g_covered_ktypes.store(covered.release(), std::memory_order_release);
    } catch (...) {
        clearShmCoveredKTypes();
    }
}

void clearShmCoveredKTypes() noexcept {
    g_covered_ktypes.store(nullptr, std::memory_order_release);
}

void registerExtraKTypeBase(const KQuery::KType& ktype, const KQuery::KType& basetype) noexcept {
    try {
        auto bases = std::make_unique<ExtraKTypeBases>();
        const auto* old = g_extra_kbase.load(std::memory_order_acquire);
        if (old) {
            bases->bases = old->bases;
        }
        bool replaced = false;
        for (auto& item : bases->bases) {
            if (equalsIgnoreCase(ktype, item.first)) {
                item.second = basetype;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            bases->bases.emplace_back(ktype, basetype);
        }
        g_extra_kbase.store(bases.release(), std::memory_order_release);
    } catch (...) {
        // An extra type whose base is unknown must not be cached in the client mode
        clearShmCoveredKTypes();
    }
}

void clearExtraKTypeBases() noexcept {
    g_extra_kbase.store(nullptr, std::memory_order_release);
}

bool isShmKTypeCovered(const KQuery::KType& ktype) noexcept {
    const auto* covered = g_covered_ktypes.load(std::memory_order_acquire);
    // Unknown coverage must not enable the cache in the client mode
    HKU_IF_RETURN(!covered, StockManager::instance().isIpcClientMode());
    for (const auto& item : covered->ktypes) {
        if (equalsIgnoreCase(ktype, item)) {
            return true;
        }
    }
    // An extra type inherits the realtime coverage of its base type
    const auto* bases = g_extra_kbase.load(std::memory_order_acquire);
    if (bases) {
        for (const auto& item : bases->bases) {
            if (equalsIgnoreCase(ktype, item.first)) {
                for (const auto& base : covered->ktypes) {
                    if (equalsIgnoreCase(item.second, base)) {
                        return true;
                    }
                }
                break;
            }
        }
    }
    return false;
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
