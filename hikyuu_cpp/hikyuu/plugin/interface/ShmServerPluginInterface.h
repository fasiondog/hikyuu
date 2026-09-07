/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/plugin/PluginBase.h"

// HKU_API 由构建以 -D 提供（核心库导出、插件导入时各不相同）；当消费方（如插件仅引用本头
// 而未先行包含 lang.h/DataType.h）未定义该宏时，回退为空，避免 class HKU_API 被当作类名解析。
// 与 lang.h / DataType.h 的同名守卫一致；Windows 插件经 -D HKU_API=__declspec(dllimport) 覆盖。
#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * 数据加载事件：插件据此在正确时点发布两类共享内存快照与刷新板块缓存
 * @details 旧形态下发布点与 refreshBlocks 调用直接埋在 StockManager 的主线程 / 预加载线程中，
 * 与进程角色协商纠缠。去 Master 化后，核心库仅按序派发本事件，插件订阅并自行决定发布时机，
 * 事件到动作的一一映射见设计文档 §5.2。
 * @ingroup DataDriver
 */
enum class LoadEvent {
    BASE_DATA_READY,         ///< 基础数据与权息就绪（历史财务尚未预加载）
    BLOCKS_LOADED,           ///< 板块数据加载完成（原 refreshBlocks）
    KDATA_PRELOAD_FINISHED,  ///< K 线预加载完成
    HISTORY_FINANCE_LOADED,  ///< 历史财务加载完成
};

/**
 * shm 数据服务插件控制接口（继承 PluginBase，与其余 *PluginInterface 同构）
 * @details 服务端整体迁入 hku_plugin/plugin_shmserver，由 Python 侧 start_shm_server() 显式拉起。
 * 核心库仅暴露本接口作为控制入口；VIP 授权门控在插件实现侧完成（复刻 dataserver 范式，见设计 §4.2）。
 * @note 本头文件仅核心库一份，插件经 add_includedirs 引用同一份、不放本地拷贝；
 * 类须 HKU_API 导出，跨 DLL dynamic_cast 才能成功（见设计 §5.4 单一来源约定）。
 * @ingroup DataDriver
 */
class HKU_API ShmServerPluginInterface : public PluginBase {
public:
    ShmServerPluginInterface() = default;
    virtual ~ShmServerPluginInterface() = default;

    /**
     * 启动 shm server
     * @param datadir 数据目录，为空时取 StockManager 当前数据目录
     * @param publish_shm 是否发布两类共享内存快照
     * @param recv_spot 是否由本进程接收实时行情（内部调用 startSpotAgent）
     * @return 启动成功返回 true；本进程已处于客户端模式时拒绝并返回 false（防自连接，见设计 §5.5）
     */
    virtual bool start(const std::string& datadir, bool publish_shm, bool recv_spot) noexcept = 0;

    /** 停止 shm server，注销镜像挂钩与加载事件回调，释放共享内存段 */
    virtual void stop() noexcept = 0;

    /** 服务是否在运行 */
    virtual bool running() const noexcept = 0;

    /** 服务监听地址（供日志 / 排障；未启动时为空串） */
    virtual const std::string& addr() const noexcept = 0;
};

}  // namespace hku
