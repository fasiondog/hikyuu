/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/plugin/PluginBase.h"

// 客户端协商能力所需的驱动类型（装配进 StockManager 的数据驱动），全部为核心库自有类型
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/KDataDriver.h"

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
 * shm 数据服务插件接口（继承 PluginBase，与其余 *PluginInterface 同构）
 * @details 服务端整体迁入 hku_plugin/plugin_shmserver，由 Python 侧 start_shm_server() 显式拉起。
 * 核心库仅依赖本接口这一份契约与插件交互，其上承载两个角色互斥的能力面：
 *   - 服务端能力（start / stop / running / addr）：由门面 startShmServer() 显式拉起服务；
 *   - 客户端协商能力（connect / serverAddr / createXxxDriver）：use_shm_server 开启且本进程非
 *     server 角色时，由 StockManager::_negotiateShmServer 接入既有服务并装配三类代理驱动。
 *
 * 客户端协商能力原独立暴露为 ipc::ShmClientInterface（插件对象多继承实现、宿主经 client() 访问器
 * 取得），现并入本接口：本接口是核心库对 shmserver 插件的**唯一**类型契约，插件的实现细节（协议
 * 编解码、共享内存布局、转发链路）一律不进入核心库——三条实时转发由插件 connect 成功后自行向
 * ipc::registerShmClient() 注册回调，核心库不再持有任何客户端接口类型。
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
