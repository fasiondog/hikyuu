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

    void showLog(bool show) {
        m_show_log = show;
    }

private:
    std::mutex m_mutex;
    std::string m_addr;
    nng_socket m_socket;
    std::atomic_bool m_connected{false};
    std::atomic_bool m_show_log{true};
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

private:
    struct Work;
    static void _serverCallback(void* arg);
    static void _processRequest(Work* work);

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
