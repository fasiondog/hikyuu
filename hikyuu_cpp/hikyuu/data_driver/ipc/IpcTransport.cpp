/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include "IpcTransport.h"
#include "hikyuu/utilities/Log.h"
#include <cstdlib>
#include <fmt/format.h>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace hku {
namespace ipc {

namespace {
std::mutex g_interrupt_checker_mutex;
std::function<bool()> g_interrupt_checker;

/**
 * 临时收发超时守卫：构造时按需覆盖 socket 超时，析构时恢复默认值。
 * 必须在持有 IpcClient::m_mutex 的临界区内使用，保证 socket 独占。
 */
class TimeoutGuard {
public:
    TimeoutGuard(nng_socket sock, uint32_t timeout_ms, uint32_t default_ms)
    : m_sock(sock), m_default_ms(default_ms), m_changed(false) {
        if (timeout_ms == default_ms) {
            return;
        }
        if (nng_socket_set_ms(sock, NNG_OPT_SENDTIMEO, timeout_ms) == 0 &&
            nng_socket_set_ms(sock, NNG_OPT_RECVTIMEO, timeout_ms) == 0) {
            m_changed = true;
        }
    }

    ~TimeoutGuard() {
        if (m_changed) {
            nng_socket_set_ms(m_sock, NNG_OPT_SENDTIMEO, m_default_ms);
            nng_socket_set_ms(m_sock, NNG_OPT_RECVTIMEO, m_default_ms);
        }
    }

    TimeoutGuard(const TimeoutGuard&) = delete;
    TimeoutGuard& operator=(const TimeoutGuard&) = delete;

private:
    nng_socket m_sock;
    uint32_t m_default_ms;
    bool m_changed;
};

/* 获取系统临时目录，用于存放服务地址与文件锁 */
std::string getIpcTempDir() {
#if defined(_WIN32)
    char buf[MAX_PATH];
    if (GetTempPathA(MAX_PATH, buf)) {
        std::string path(buf);
        while (!path.empty() && (path.back() == '\\' || path.back() == '/')) {
            path.pop_back();
        }
        HKU_IF_RETURN(!path.empty(), path);
    }
    return ".";
#else
    const char* tmp = std::getenv("TMPDIR");
    return (tmp && *tmp) ? std::string(tmp) : "/tmp";
#endif
}
}  // namespace

void setInterruptChecker(std::function<bool()> checker) {
    std::lock_guard<std::mutex> lock(g_interrupt_checker_mutex);
    g_interrupt_checker = std::move(checker);
}

bool checkInterrupted() {
    std::function<bool()> checker;
    {
        std::lock_guard<std::mutex> lock(g_interrupt_checker_mutex);
        checker = g_interrupt_checker;
    }
    // 检查器在锁外执行（Python 环境下需获取 GIL，锁内执行可能死锁）
    return checker ? checker() : false;
}

void makeIpcServerPaths(const std::string& datadir, std::string& addr, std::string& lock_path) {
    size_t h = std::hash<std::string>()(datadir);
    std::string ipc_dir = getIpcTempDir();
#if defined(_WIN32)
    // Windows: nng 将 ipc:// 映射为命名管道 \\.\pipe\<name>，管道名不能含盘符或反斜杠，
    // 否则 CreateNamedPipeA 必然失败。隔离性已由 datadir 哈希保证，故仅用不含目录的固定名。
    addr = fmt::format("ipc://hikyuu_kdata_server_{:x}", h);
#else
    // POSIX: ipc:// 为真实文件系统路径（Unix domain socket），须落在可写的临时目录下
    addr = fmt::format("ipc://{}/hikyuu_kdata_server_{:x}.ipc", ipc_dir, h);
#endif
    lock_path = fmt::format("{}/hikyuu_kdata_server_{:x}.lock", ipc_dir, h);
}

///////////////////////////////////////////////////////////////////////////////
// IpcClient
///////////////////////////////////////////////////////////////////////////////
IpcClient::IpcClient(const std::string& addr) : m_addr(addr) {}

IpcClient::~IpcClient() {
    close();
}

bool IpcClient::dial() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    HKU_IF_RETURN(m_addr.empty(), false);
    if (m_connected) {
        nng_close(m_socket);
        m_connected = false;
    }

    int rv = nng_req0_open(&m_socket);
    HKU_IF_RETURN(rv != 0, false);
    m_connected = true;

    try {
        rv = nng_socket_set_ms(m_socket, NNG_OPT_RECONNMINT, 10);
        HKU_CHECK(rv == 0, "Failed nng_socket_set_ms! {}", nng_strerror(rv));

        rv = nng_socket_set_ms(m_socket, NNG_OPT_RECONNMAXT, 15000);
        HKU_CHECK(rv == 0, "Failed nng_socket_set_ms! {}", nng_strerror(rv));

        rv = nng_socket_set_ms(m_socket, NNG_OPT_SENDTIMEO, m_timeout_ms);
        HKU_CHECK(rv == 0, "Failed nng_socket_set_ms! {}", nng_strerror(rv));

        rv = nng_socket_set_ms(m_socket, NNG_OPT_RECVTIMEO, m_timeout_ms);
        HKU_CHECK(rv == 0, "Failed nng_socket_set_ms! {}", nng_strerror(rv));

        rv = nng_dial(m_socket, m_addr.c_str(), NULL, 0);
        HKU_CHECK(rv == 0, "Failed dial ipc server: {}! {}", m_addr, nng_strerror(rv));
        return true;

    } catch (const std::exception& e) {
        // 拨号失败是主从协商的正常分支（首个进程探测不到已有服务时才转为主进程），
        // 协商失败已由 StockManager 统一告警，此处仅留调试级线索，避免单进程启动即报 ERROR
        HKU_DEBUG_IF(m_show_log, "Failed dial ipc server: {}! {}", m_addr, e.what());
    } catch (...) {
        HKU_DEBUG_IF(m_show_log, "Failed dial ipc server: {}! Unknown error!", m_addr);
    }

    m_connected = false;
    nng_close(m_socket);
    return false;
}

void IpcClient::close() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_connected) {
        nng_close(m_socket);
        m_connected = false;
    }
}

bool IpcClient::request(const std::vector<uint8_t>& request_frame,
                        std::vector<uint8_t>& response_frame) noexcept {
    return request(request_frame, response_frame, m_timeout_ms);
}

bool IpcClient::request(const std::vector<uint8_t>& request_frame,
                        std::vector<uint8_t>& response_frame, uint32_t timeout_ms) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    HKU_IF_RETURN(!m_connected, false);

    nng_msg* msg = nullptr;
    int rv = nng_msg_alloc(&msg, 0);
    HKU_IF_RETURN(rv != 0, false);

    bool sent = false;
    bool success = false;
    nng_msg* res_msg = nullptr;
    bool res_owned = false;  // 已收到响应但尚未释放
    // response_frame.assign 可能因内存不足抛异常，而本函数承诺 noexcept，
    // 不在此兜住会直接 terminate；异常一律视为通讯失败，由调用方回退本地驱动
    try {
        rv = nng_msg_append(msg, request_frame.data(), request_frame.size());
        if (rv == 0) {
            // 持锁期间独占 socket，可安全地临时调整超时，离开临界区前自动恢复
            TimeoutGuard guard(m_socket, timeout_ms, m_timeout_ms);
            rv = nng_sendmsg(m_socket, msg, 0);
            if (rv == 0) {
                // nng_sendmsg 成功后消息所有权即转移给 nng，不得再释放
                sent = true;
                rv = nng_recvmsg(m_socket, &res_msg, 0);
                if (rv == 0) {
                    res_owned = true;
                    const uint8_t* body = (const uint8_t*)nng_msg_body(res_msg);
                    response_frame.assign(body, body + nng_msg_len(res_msg));
                    success = true;
                } else {
                    HKU_ERROR_IF(m_show_log, "Failed recv ipc response! {}", nng_strerror(rv));
                }
            } else {
                HKU_ERROR_IF(m_show_log, "Failed send ipc request! {}", nng_strerror(rv));
            }
        }
    } catch (const std::exception& e) {
        HKU_ERROR_IF(m_show_log, "Failed ipc request! {}", e.what());
        success = false;
    } catch (...) {
        HKU_ERROR_IF(m_show_log, "Failed ipc request! Unknown error!");
        success = false;
    }

    // assign 抛异常时响应消息仍由本函数持有，须在此释放，避免泄漏
    if (res_owned) {
        nng_msg_free(res_msg);
    }
    if (!sent) {
        nng_msg_free(msg);
    }
    return success;
}

///////////////////////////////////////////////////////////////////////////////
// IpcServer
///////////////////////////////////////////////////////////////////////////////
struct IpcServer::Work {
    enum State { INIT, RECV, SEND, FINISH } state = INIT;
    nng_aio* aio{nullptr};
    nng_ctx ctx;
    bool ctx_valid{false};
    IpcServer* server{nullptr};
};

IpcServer::IpcServer(const std::string& addr) : m_addr(addr) {}

IpcServer::~IpcServer() {
    stop();
}

void IpcServer::start(size_t max_parallel) {
    HKU_CHECK(!m_addr.empty(), "You must set IpcServer's addr first!");
    HKU_CHECK(m_handler, "You must set IpcServer's handler first!");
    HKU_IF_RETURN(m_running, void());

    int rv = nng_rep0_open(&m_socket);
    HKU_CHECK(0 == rv, "Failed open ipc server socket! {}", nng_strerror(rv));

    bool listened = false;
    try {
        rv = nng_listen(m_socket, m_addr.c_str(), &m_listener, 0);
        HKU_CHECK(0 == rv, "Failed listen ipc server socket ({})! {}", m_addr, nng_strerror(rv));
        listened = true;

        m_works.reserve(max_parallel);
        for (size_t i = 0; i < max_parallel; i++) {
            Work* w = new Work();
            m_works.push_back(w);
            rv = nng_aio_alloc(&w->aio, _serverCallback, w);
            HKU_CHECK(0 == rv, "Failed create ipc work {}! {}", i, nng_strerror(rv));
            rv = nng_ctx_open(&w->ctx, m_socket);
            HKU_CHECK(0 == rv, "Failed open ipc ctx {}! {}", i, nng_strerror(rv));
            w->ctx_valid = true;
            w->state = Work::INIT;
            w->server = this;
        }

        m_running = true;
        for (auto* w : m_works) {
            _serverCallback(w);
        }
        HKU_INFO("Ipc data server started, listen: {}", m_addr);
    } catch (...) {
        if (m_running) {
            // 回调可能已在飞，必须走 stop() 的安全路径（nng_aio_stop 等待回调结束后再释放），
            // 否则会与正在执行的 _serverCallback 并发读写 Work 成员
            stop();
        } else {
            // 启动失败时清理已创建的资源，避免泄漏（此时回调尚未启动，无需 stop aio）
            for (auto* w : m_works) {
                if (w->ctx_valid) {
                    nng_ctx_close(w->ctx);
                }
                if (w->aio) {
                    nng_aio_free(w->aio);
                }
                delete w;
            }
            m_works.clear();
            if (listened) {
                nng_listener_close(m_listener);
            }
            nng_close(m_socket);
        }
        throw;
    }
}

void IpcServer::stop() {
    HKU_IF_RETURN(!m_running, void());
    m_running = false;

    for (auto* w : m_works) {
        if (w->aio) {
            // 先等待回调结束，避免与回调并发读写 Work 成员造成数据竞争
            nng_aio_stop(w->aio);
            w->server = nullptr;
            w->state = Work::FINISH;
            nng_aio_free(w->aio);
            if (w->ctx_valid) {
                nng_ctx_close(w->ctx);
            }
            w->aio = nullptr;
        }
        delete w;
    }
    m_works.clear();

    nng_listener_close(m_listener);
    nng_close(m_socket);
    HKU_INFO("Ipc data server stopped.");
}

void IpcServer::_serverCallback(void* arg) {
    Work* work = static_cast<Work*>(arg);
    int rv = 0;
    switch (work->state) {
        case Work::INIT:
            work->state = Work::RECV;
            nng_ctx_recv(work->ctx, work->aio);
            break;

        case Work::RECV:
            _processRequest(work);
            break;

        case Work::SEND:
            if ((rv = nng_aio_result(work->aio)) != 0) {
                HKU_ERROR_IF(rv != NNG_ECANCELED && rv != NNG_ECLOSED,
                             "Failed ipc server ctx send! {}", nng_strerror(rv));
            }
            _rearm(work);
            break;

        case Work::FINISH:
            break;

        default:
            HKU_ERROR("Ipc server bad state!");
            break;
    }
}

void IpcServer::_rearm(Work* work) {
    // aio 可能仍持有未完成发送的消息，nng_ctx_recv 完成时会直接覆盖它，
    // 故先摘出再释放；不可就地 free（aio 仍持有该指针，stop() 时会被二次处理）
    if (nng_msg* pending = nng_aio_get_msg(work->aio)) {
        nng_aio_set_msg(work->aio, nullptr);
        nng_msg_free(pending);
    }

    // 客户端在响应传输期间断开（强杀、超时放弃）属常态，若就此置 FINISH，
    // 该 ctx 将永久退役且从不重建，长期运行后并发槽单调耗尽、服务实质不可用。
    // 仅在服务停止（stop 已先置 m_running=false 再 nng_aio_stop）时结束 worker。
    if (!work->server || !work->server->running() || !work->ctx_valid) {
        work->state = Work::FINISH;
        return;
    }
    work->state = Work::RECV;
    nng_ctx_recv(work->ctx, work->aio);
}

void IpcServer::_processRequest(Work* work) {
    IpcServer* server = work->server;
    HKU_IF_RETURN(!server || !work->aio, void());
    nng_msg* msg = nullptr;

    std::vector<uint8_t> response_frame;
    try {
        int rv = nng_aio_result(work->aio);
        if (rv != 0) {
            HKU_ERROR_IF(rv != NNG_ECANCELED && rv != NNG_ECLOSED, "Failed nng_aio_result! {}",
                         nng_strerror(rv));
            _rearm(work);
            return;
        }

        msg = nng_aio_get_msg(work->aio);
        const uint8_t* body = (const uint8_t*)nng_msg_body(msg);
        std::vector<uint8_t> request_frame(body, body + nng_msg_len(msg));

        Cmd cmd;
        std::vector<uint8_t> request_body;
        if (!decodeRequest(request_frame, cmd, request_body)) {
            Encoder enc;
            enc.putString("Invalid request frame!");
            response_frame = encodeResponse(RetCode::ERROR, enc.data());
        } else {
            RetCode ret = RetCode::SUCCESS;
            std::vector<uint8_t> response_body;
            try {
                response_body = server->m_handler(cmd, std::move(request_body), ret);
            } catch (const std::exception& e) {
                ret = RetCode::ERROR;
                Encoder enc;
                enc.putString(e.what());
                response_body = enc.data();
            } catch (...) {
                ret = RetCode::ERROR;
                Encoder enc;
                enc.putString("Unknown error!");
                response_body = enc.data();
            }
            response_frame = encodeResponse(ret, response_body);
        }

        nng_msg_clear(msg);
        rv = nng_msg_append(msg, response_frame.data(), response_frame.size());
        HKU_CHECK(rv == 0, "Failed nng_msg_append!");

        nng_aio_set_msg(work->aio, msg);
        work->state = Work::SEND;
        nng_ctx_send(work->ctx, work->aio);

    } catch (const std::exception& e) {
        HKU_ERROR("Ipc server process request failed! {}", e.what());
        // 不在此处 nng_msg_free：msg 取自 nng_aio_get_msg，aio 仍持有该指针，
        // 就地释放会让后续 nng_aio_free 面对已释放内存；由 _rearm 摘出后统一释放
        _rearm(work);

    } catch (...) {
        HKU_ERROR("Ipc server process request failed! Unknown error!");
        _rearm(work);
    }
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
