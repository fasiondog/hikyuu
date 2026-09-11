/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include <atomic>
#include "hikyuu/StockManager.h"
#include "interface/plugins.h"
#include "shmserver.h"

namespace hku {

namespace {
// 门面持有"本进程已启动"的服务插件指针：stop / isRunning 仅查询此指针，绝不走 getPlugin 的按需加载。
// 原因：stopShmServer() 由 GlobalInitializer::clean() 在每次进程退出时无条件调用，若经 getPlugin
// 会对未安装 / 本未启动 shmserver 的普通独立进程反复尝试 dlopen 并误报，污染退出路径。
std::atomic<ShmServerPluginInterface*> g_shm_server_plugin{nullptr};
}  // namespace

bool HKU_API startShmServer(const std::string& datadir, bool publish_shm, bool recv_spot) noexcept {
    // 先置 server 角色：即便 StockManager 尚未 init，_negotiateShmServer() 亦据此跳过客户端协商，
    // 防止服务进程误入客户端模式形成 realtimeUpdate 自转发环（见设计 §5.5）
    setShmServerRole(true);

    // 幂等：本进程已启动服务则直接返回成功
    HKU_INFO_IF_RETURN(g_shm_server_plugin.load(std::memory_order_acquire) != nullptr, true,
                       "hikyuu shm server is already running.");

    StockManager& sm = StockManager::instance();
    ShmServerPluginInterface* plugin = sm.getPlugin<ShmServerPluginInterface>(HKU_PLUGIN_SHM_SERVER);
    HKU_ERROR_IF_RETURN(!plugin, false,
                        "Can't find {} plugin! It is a VIP plugin; check that it is installed and "
                        "properly licensed.",
                        HKU_PLUGIN_SHM_SERVER);

    // 插件 start() 内部负责：授权校验、自连接守卫（isIpcClientMode 为真即拒绝）、
    // 注册 ShmMirrorSink 与 LoadEvent 回调、startSpotAgent（recv_spot）、首次发布
    HKU_ERROR_IF_RETURN(!plugin->start(datadir, publish_shm, recv_spot), false,
                        "Failed to start hikyuu shm server plugin!");

    g_shm_server_plugin.store(plugin, std::memory_order_release);
    return true;
}

void HKU_API stopShmServer() noexcept {
    // 取出并清空指针：exchange 保证 stop 幂等，且此后 isRunning 立即反映为未运行
    ShmServerPluginInterface* plugin = g_shm_server_plugin.exchange(nullptr, std::memory_order_acq_rel);
    HKU_IF_RETURN(!plugin, void());
    // 插件 stop() 内部按序：停行情、注销 ShmMirrorSink、注销 LoadEvent 回调、ShmServer::stop(true)、销毁发布器
    plugin->stop();
}

bool HKU_API isShmServerRunning() noexcept {
    ShmServerPluginInterface* plugin = g_shm_server_plugin.load(std::memory_order_acquire);
    return plugin ? plugin->running() : false;
}

}  // namespace hku
