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

// 转发回调对象：注册时在堆上创建并以 release 发布，转发侧 acquire 读取后只读访问；对象进程
// 生命周期内常驻、不释放，以避免与行情线程（转发读方）产生释放-使用竞争（注册仅发生一两次）
std::atomic<ShmClientForwarders*> g_client{nullptr};

// 中断检查器由插件侧等待逻辑（运行期）与 Python 绑定（注册）共同使用；堆分配且永不释放，
// 避免静态析构期（如插件 stop 路径再触发一次检查）触碰已销毁的 TU 级静态量
std::mutex* g_interrupt_checker_mutex = new std::mutex;
std::function<bool()>* g_interrupt_checker = new std::function<bool()>;

}  // namespace

void registerShmClient(ShmClientForwarders fwd) noexcept {
    if (!fwd) {
        // 注销：切断引用，转发侧随即按安全空操作处理
        g_client.store(nullptr, std::memory_order_release);
        return;
    }
    // 发布新回调组（不再释放，见上）
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
    // 检查器在锁外执行（Python 环境下需获取 GIL，锁内执行可能死锁）
    return checker ? checker() : false;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
