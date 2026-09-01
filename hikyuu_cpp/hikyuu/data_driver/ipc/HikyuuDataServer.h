/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include "IpcTransport.h"

namespace hku {
namespace ipc {

class FileLock;
class KDataShmPublisher;

/**
 * 单机 IPC 数据服务（Master 端）
 * @details 第一个启动的 hikyuu 进程通过文件锁竞争成为 Master，
 * 以 nng REP 方式向同机其他进程提供基础信息与 K 线数据服务。
 * @ingroup DataDriver
 */
class HKU_API HikyuuDataServer {
public:
    HikyuuDataServer();
    ~HikyuuDataServer();

    HikyuuDataServer(const HikyuuDataServer&) = delete;
    HikyuuDataServer& operator=(const HikyuuDataServer&) = delete;

    /**
     * 尝试获取独占文件锁并启动数据服务
     * @param addr 监听地址，如 "ipc:///tmp/hikyuu_kdata_server.ipc"
     * @param lock_path 主从选举文件锁路径
     * @param tmpdir 临时目录（当前未使用，保留）
     * @return true 竞争成功，本进程为 Master | false 已有其他进程提供服务
     */
    bool start(const std::string& addr, const std::string& lock_path, const std::string& tmpdir);

    /** 停止服务并释放文件锁 */
    void stop();

    bool running() const noexcept {
        return m_running;
    }

    const std::string& addr() const noexcept {
        return m_addr;
    }

    /**
     * 更新 K 线预加载进度
     * @param loaded 已加载证券数
     * @param total 证券总数
     */
    void updateProgress(uint64_t loaded, uint64_t total);

    /** 全部数据就绪 */
    void setAllReady() noexcept;

    bool allReady() const noexcept {
        return m_all_ready;
    }

    /** 刷新板块缓存（数据加载/重加载完成后调用） */
    void refreshBlocks();

    /**
     * 将预加载的 K 线缓冲发布为只读共享内存缓存快照，供客户端零拷贝读取
     * @param shm_name_prefix 共享内存段名前缀（受系统名称长度限制，建议不超过 11 字符）
     * @return true 发布成功
     */
    bool publishShmCache(const std::string& shm_name_prefix);

private:
    std::vector<uint8_t> _handle(Cmd cmd, std::vector<uint8_t>&& body, RetCode& ret);

private:
    std::string m_addr;
    IpcServer m_server;
    std::unique_ptr<FileLock> m_lock;
    std::atomic_bool m_running{false};
    std::atomic_bool m_all_ready{false};
    std::atomic<uint64_t> m_kloaded{0};
    std::atomic<uint64_t> m_ktotal{0};

    std::shared_mutex m_block_mutex;
    BlockList m_blocks;

    mutable std::shared_mutex m_shm_mutex;  // 保护 m_shm_name / m_shm_epoch
    std::string m_shm_name;
    uint64_t m_shm_epoch{0};
    // 发布器生命周期与服务一致：持有当前段，重发布时自动删旧段，服务销毁时清理；
    // 仅在后台预加载线程中访问，无需加锁保护
    std::unique_ptr<KDataShmPublisher> m_shm_publisher;
};

typedef std::shared_ptr<HikyuuDataServer> HikyuuDataServerPtr;

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
