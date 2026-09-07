/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <atomic>
#include <functional>
#include <mutex>
#include "ShmClientHook.h"

namespace hku {
namespace ipc {

namespace {

std::atomic<ShmClientInterface*> g_client{nullptr};

// 中断检查器由插件侧等待逻辑（运行期）与 Python 绑定（注册）共同使用；堆分配且永不释放，
// 避免静态析构期（如插件 stop 路径再触发一次检查）触碰已销毁的 TU 级静态量
std::mutex* g_interrupt_checker_mutex = new std::mutex;
std::function<bool()>* g_interrupt_checker = new std::function<bool()>;

}  // namespace

void registerShmClient(ShmClientInterface* client) noexcept {
    g_client.store(client, std::memory_order_release);
}

ShmClientInterface* shmClient() noexcept {
    return g_client.load(std::memory_order_acquire);
}

bool forwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                           const KRecord& record) {
    ShmClientInterface* client = g_client.load(std::memory_order_acquire);
    HKU_IF_RETURN(!client, false);
    return client->forwardRealtimeUpdate(market_code, ktype, record);
}

Datetime forwardGetLastUpdateTime(const std::string& market_code, const KQuery::KType& ktype) {
    ShmClientInterface* client = g_client.load(std::memory_order_acquire);
    HKU_IF_RETURN(!client, Datetime::min());
    return client->forwardGetLastUpdateTime(market_code, ktype);
}

bool forwardPullFromBufferServer(const std::string& addr, const std::vector<std::string>& codes,
                                 const KQuery::KType& ktype) {
    ShmClientInterface* client = g_client.load(std::memory_order_acquire);
    HKU_IF_RETURN(!client, false);
    return client->forwardPullFromBufferServer(addr, codes, ktype);
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
    // 检查器在锁外执行（Python 环境下需获取 GIL，锁内执行可能死锁）
    return checker ? checker() : false;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
