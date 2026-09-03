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
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <nng/nng.h>
#include "IpcProtocol.h"

namespace hku {
namespace ipc {

/**
 * 设置长阻塞等待期间的中断检查器（如等待数据服务就绪）。
 * 返回 true 表示应中断等待。Python 环境下由绑定层注册，用于响应 Ctrl+C 信号，
 * 避免客户端进程在等待期间长时间不可中断。
 */
HKU_API void setInterruptChecker(std::function<bool()> checker);

/** 调用已注册的中断检查器，未注册时返回 false */
HKU_API bool checkInterrupted();

/**
 * 构造单机 IPC 数据服务的通讯地址与主进程竞选文件锁路径
 * @details 以 datadir 哈希隔离不同数据集/项目，避免互扰。平台差异：
 * - POSIX：ipc:// 是真实文件系统路径（Unix domain socket），落在系统临时目录下；
 * - Windows：nng 将 ipc:// 映射为命名管道 \\.\pipe\<name>，管道名不能含盘符或反斜杠
 *   （否则 CreateNamedPipeA 必然失败），故仅用不含目录的固定名 + datadir 哈希。
 * 生产代码与单元测试共用本函数，避免两侧地址构造方式不一致而漏测平台问题。
 * @param datadir 数据目录，用于派生隔离哈希
 * @param addr [out] nng ipc:// 通讯地址（master 监听与 client 拨号共用）
 * @param lock_path [out] 主进程竞选文件锁路径
 */
HKU_API void makeIpcServerPaths(const std::string& datadir, std::string& addr,
                                std::string& lock_path);

/**
 * IPC 数据服务客户端（nng REQ，二进制帧）
 * @ingroup DataDriver
 */
class HKU_API IpcClient {
public:
    IpcClient() = default;
    explicit IpcClient(const std::string& addr);
    ~IpcClient();

    IpcClient(const IpcClient&) = delete;
    IpcClient& operator=(const IpcClient&) = delete;

    void setAddr(const std::string& addr) {
        m_addr = addr;
    }

    /** 连接服务端 */
    bool dial() noexcept;

    /** 关闭连接 */
    void close() noexcept;

    bool connected() const noexcept {
        return m_connected;
    }

    /**
     * 发送请求帧并接收响应帧（原始帧，含帧头）
     * @param request_frame 请求帧（encodeRequest 的结果）
     * @param response_frame [out] 响应帧（供 decodeResponse 解析）
     */
    bool request(const std::vector<uint8_t>& request_frame,
                 std::vector<uint8_t>& response_frame) noexcept;

    /**
     * 同上，但为本次请求临时指定收发超时，用于轻量探测类请求（如快照协商），
     * 避免服务端不可用（如定时重启窗口）时长时间阻塞调用方。
     * 超时值在持锁期间设置并于返回前恢复，不影响其他请求。
     * @param timeout_ms 本次请求的发送/接收超时（毫秒）
     */
    bool request(const std::vector<uint8_t>& request_frame, std::vector<uint8_t>& response_frame,
                 uint32_t timeout_ms) noexcept;

    void showLog(bool show) {
        m_show_log = show;
    }

private:
    std::mutex m_mutex;
    std::string m_addr;
    nng_socket m_socket;
    std::atomic_bool m_connected{false};
    std::atomic_bool m_show_log{true};
    uint32_t m_timeout_ms{10000};  ///< 默认收发超时，构造后不变
};

typedef std::shared_ptr<IpcClient> IpcClientPtr;

/**
 * IPC 数据服务端（nng REP，二进制帧，多 ctx 并发）
 * @ingroup DataDriver
 */
class HKU_API IpcServer {
public:
    /**
     * 命令处理器
     * @param cmd 命令字
     * @param body 请求体
     * @param ret [out] 响应状态，默认 SUCCESS
     * @return 响应体
     */
    typedef std::function<std::vector<uint8_t>(Cmd, std::vector<uint8_t>&&, RetCode&)> Handler;

    IpcServer() = default;
    explicit IpcServer(const std::string& addr);
    ~IpcServer();

    IpcServer(const IpcServer&) = delete;
    IpcServer& operator=(const IpcServer&) = delete;

    void setAddr(const std::string& addr) {
        m_addr = addr;
    }

    void setHandler(Handler handle) {
        m_handler = std::move(handle);
    }

    void start(size_t max_parallel = 128);
    void stop();

    bool running() const noexcept {
        return m_running;
    }

    /// 仅置停止标志而不触碰 nng：用于 Windows 进程退出路径，跳过对在飞异步 recv 的
    /// 阻塞式取消（命名管道 teardown 在静态析构期会陷入内核态不可中断等待）；
    /// 在飞回调随后经 _rearm 检测 running()==false 自然收敛为 FINISH，不再重投递。
    void markStopped() noexcept {
        m_running = false;
    }

private:
    struct Work;
    static void _serverCallback(void* arg);
    static void _processRequest(Work* work);

    /**
     * 复用该并发槽：释放 aio 残留消息后重新装载接收
     * @details 传输层错误（客户端在响应期间断开）不应让 worker 永久退役，
     * 否则并发槽单调耗尽；仅在服务已停止时置 FINISH。
     */
    static void _rearm(Work* work);

    std::string m_addr;
    nng_socket m_socket;
    nng_listener m_listener;
    std::vector<Work*> m_works;
    Handler m_handler;
    std::atomic_bool m_running{false};
};

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
