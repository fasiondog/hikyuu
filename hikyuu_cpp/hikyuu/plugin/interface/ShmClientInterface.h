/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <string>
#include <vector>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/KDataDriver.h"

namespace hku {
namespace ipc {

/**
 * shm 客户端能力契约（由 shmserver 插件实现，核心库仅持裸指针调用）
 * @details 单机共享内存数据服务的**全部**实现（协议编解码、nng 传输、段布局与读写、
 * 客户端代理驱动、转发链路）均在 shmserver 插件内；核心库不链接其中任何符号，只在客户端
 * 协商时经本接口向插件取回代理驱动并转发三条实时链路，从而在"未安装 / 未授权插件"时
 * 天然退化为独立模式（自行加载全部数据，行为与未启用该特性完全一致）。
 *
 * 装配职责划分：
 *   - 插件：connect（探测 + 等待就绪 + 重试）、创建三类代理驱动、维护连接与转发状态；
 *   - 核心库：use_shm_server 门控、把驱动装配进 StockManager、置客户端模式标志、
 *     关闭本地预加载、在退出期注销。
 *
 * @note 本接口与 ShmServerPluginInterface 同属 shmserver 插件接口族、同居本目录（单一来源，
 * 插件经 add_includedirs 引用同一份、不放本地拷贝）。区别于继承 PluginBase 的控制接口，
 * 本接口是插件对象暴露给宿主的**客户端能力面**：宿主在协商时经
 * ShmServerPluginInterface::client() 取得本接口，不经裸 dynamic_cast。
 * 生命周期由插件侧保证：实现对象随插件实例存活（PluginManager 在进程退出前不卸载），
 * 核心库不持有所有权，仅在退出路径经 registerShmClient(nullptr) 断开引用。
 * @ingroup DataDriver
 */
class HKU_API ShmClientInterface {
public:
    virtual ~ShmClientInterface() = default;

    /**
     * 探测并连接既有 shm 服务，阻塞等待其数据就绪
     * @param datadir 数据目录（服务地址由其哈希派生，保证只有同数据集的进程互连）
     * @param wait_timeout_sec 等待就绪的总预算（秒），0 表示无限等待
     * @return true 已连接且服务就绪（本进程应进入客户端模式）| false 无服务/超时/被中断
     * @note 实现内部负责连接重试与中断检查（经核心库 ipc::checkInterrupted()），
     * 且**不得**在本进程拉起服务——服务只能由 start_shm_server() 显式启动
     */
    virtual bool connect(const std::string& datadir, uint64_t wait_timeout_sec) noexcept = 0;

    /**
     * 已连接的服务地址（日志用）
     * @note 刻意不命名为 addr()：插件实现类同时继承服务端的 ShmServerPluginInterface
     * （其 addr() 返回 const std::string&），同名不同返回类型将使两个 override 互相隐藏
     */
    virtual std::string serverAddr() const noexcept = 0;

    /** 创建 K 线代理驱动（内部含本地驱动连接池，用于本地优先与降级兜底） */
    virtual KDataDriverPtr createKDataDriver(
      const KDataDriverConnectPoolPtr& local_pool) noexcept = 0;

    /** 创建基础信息代理驱动（local 为降级兜底的本地驱动） */
    virtual BaseInfoDriverPtr createBaseInfoDriver(const BaseInfoDriverPtr& local) noexcept = 0;

    /** 创建板块代理驱动（local 为降级兜底的本地驱动） */
    virtual BlockInfoDriverPtr createBlockDriver(const BlockInfoDriverPtr& local) noexcept = 0;

    /**
     * 客户端实时更新转发（Stock::realtimeUpdate 在客户端且本地无缓冲时调用）
     * @details 客户端无预加载缓冲，更新转由服务进程应用到其缓冲并镜像共享内存，
     * 全体客户端由此读到该更新（与服务端自身行情接收幂等收敛）
     */
    virtual bool forwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                       const KRecord& record) noexcept = 0;

    /** 客户端查询服务进程缓冲的最后更新时刻（Stock::getLastUpdateTime 调用） */
    virtual Datetime forwardGetLastUpdateTime(const std::string& market_code,
                                              const KQuery::KType& ktype) noexcept = 0;

    /** 客户端委托服务进程从行情缓存服务（buffer server）拉取最新 K 线 */
    virtual bool forwardPullFromBufferServer(const std::string& addr,
                                             const std::vector<std::string>& codes,
                                             const KQuery::KType& ktype) noexcept = 0;

    /** 断开连接并释放内部状态（注销转发器、关闭 nng 连接）；须为幂等 */
    virtual void disconnect() noexcept = 0;
};

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
