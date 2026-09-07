/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include "ShmMirrorSink.h"
#include <atomic>
#include <cstdint>

// 取当前进程 id，用于识别 fork 出的子进程（Windows 无 fork，仅为统一代码路径）
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
 * 已注册的镜像实现 + 注册进程 pid
 * @details 原子快路径：未注册（客户端进程 / 未启动 shmserver）时仅一次 acquire load 即返回。
 * pid 门控：Linux fork 的子进程继承段映射与 sink 指针，若子进程也触发 Stock::realtimeUpdate，
 * 同一 entry 将出现第二个写者，破坏 seqlock 单写者前提并致读端撕裂；仅注册进程（pid 相符）转发。
 * sink 由插件在 start / stop 单点注册 / 注销。核心库不做任何暂存——发布窗口的暂存 / 旧段双写 /
 * 接管后重放均属插件侧 ShmMirrorSink 实现的内部职责（见设计 §5.1）。
 * @note g_sink / g_sink_pid 为平凡可析构的 namespace 级原子量，无退出期锁销毁顺序问题
 */
std::atomic<ShmMirrorSink*> g_sink{nullptr};
std::atomic<int64_t> g_sink_pid{0};

}  // namespace

void registerShmMirrorSink(ShmMirrorSink* sink) noexcept {
    // 先落 pid 再落指针：注销时置 nullptr 后 pid 归零；register 由插件单线程 start/stop 调用
    g_sink_pid.store(sink ? (int64_t)HKU_SHM_GETPID() : 0, std::memory_order_relaxed);
    g_sink.store(sink, std::memory_order_release);
}

void shmMirrorRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                             const KRecord& record) {
    ShmMirrorSink* sink = g_sink.load(std::memory_order_acquire);
    if (!sink) {
        return;
    }
    // fork 出的子进程不是注册 sink 的进程，写入会成为双写者；
    // 此处仅注册进程会执行到，getpid 的 syscall 开销相对行情更新频率可忽略
    if (g_sink_pid.load(std::memory_order_relaxed) != (int64_t)HKU_SHM_GETPID()) {
        return;
    }
    sink->onRealtimeUpdate(market_code, ktype, record);
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
