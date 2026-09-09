/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/plugin/PluginBase.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/KDataDriver.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * 数据加载事件：插件据此在正确时点发布两类共享内存快照与刷新板块缓存
 * @ingroup DataDriver
 */
enum class LoadEvent {
    BASE_DATA_READY,         ///< 基础数据与权息就绪（历史财务尚未预加载）
    BLOCKS_LOADED,           ///< 板块数据加载完成
    KDATA_PRELOAD_FINISHED,  ///< K 线预加载完成
    HISTORY_FINANCE_LOADED,  ///< 历史财务加载完成
};

/**
 * shm 数据服务插件接口
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

    // ── 客户端协商能力（原 ShmClientInterface 契约并入本接口）─────────────────────────
    // 仅供 StockManager 客户端协商路径调用（use_shm_server 开启且本进程非 server 角色）。
    // 签名仅涉及核心库数据类型，接口中不出现任何插件私有类型。

    /**
     * 探测并连接既有 shm 服务，阻塞等待其数据就绪
     * @param datadir 数据目录（服务地址由其哈希派生，保证只有同数据集的进程互连）
     * @param wait_timeout_sec 等待就绪的总预算（秒），0 表示无限等待
     * @return true 已连接且服务就绪（本进程应进入客户端模式）| false 无服务/超时/被中断
     * @note 实现内部负责连接重试与中断检查（经核心库 ipc::checkInterrupted()），且**不得**在本
     * 进程拉起服务——服务只能由 start_shm_server() 显式启动；连接成功后须自行向核心库
     * ipc::registerShmClient() 注册三条转发回调（此后 Stock::realtimeUpdate 等经核心库薄转发层
     * 转至服务进程），并在断开 / 销毁时注销。
     */
    virtual bool connect(const std::string& datadir, uint64_t wait_timeout_sec) noexcept = 0;

    /** 已连接的服务地址（日志用） */
    virtual std::string serverAddr() const noexcept = 0;

    /** 创建 K 线代理驱动（内部含本地驱动连接池，用于本地优先与降级兜底） */
    virtual KDataDriverPtr createKDataDriver(
      const KDataDriverConnectPoolPtr& local_pool) noexcept = 0;

    /** 创建基础信息代理驱动（local 为降级兜底的本地驱动） */
    virtual BaseInfoDriverPtr createBaseInfoDriver(const BaseInfoDriverPtr& local) noexcept = 0;

    /** 创建板块代理驱动（local 为降级兜底的本地驱动） */
    virtual BlockInfoDriverPtr createBlockDriver(const BlockInfoDriverPtr& local) noexcept = 0;
};

}  // namespace hku
