/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-04-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"
#if !HKU_ENABLE_NODE
#error "Don't enable node server, please config with --node=y"
#endif

#include <string>
#include <unordered_map>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>

#include <hikyuu/utilities/osdef.h>
#if HKU_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WS2tcpip.h>
#include <Ws2ipdef.h>
#else
#include <arpa/inet.h>
#endif

#include "hikyuu/utilities/Log.h"
#include "NodeMessage.h"

namespace hku {

class NodeServer {
    CLASS_LOGGER_IMP(NodeServer)

public:
    NodeServer() = default;
    explicit NodeServer(const std::string& addr) : m_addr(addr) {}
    virtual ~NodeServer() {
        stop();
    }

    void setAddr(const std::string& addr) {
        m_addr = addr;
    }

    void regHandle(const std::string& cmd, const std::function<json(json&& req)>& handle) {
        m_handles[cmd] = handle;
    }

    void regHandle(const std::string& cmd, std::function<json(json&& req)>&& handle) {
        m_handles[cmd] = std::move(handle);
    }

    void start(size_t max_parrel = 128) {
        CLS_CHECK(!m_addr.empty(), "You must set NodeServer's addr first!");

        // 启动 node server
        int rv = nng_rep0_open(&m_socket);
        CLS_CHECK(0 == rv, "Failed open server socket! {}", nng_strerror(rv));
        rv = nng_listen(m_socket, m_addr.c_str(), &m_listener, 0);
        CLS_CHECK(0 == rv, "Failed listen node server socket ({})! {}", m_addr, nng_strerror(rv));
        CLS_TRACE("channel lisenter server: {}", m_addr);

        m_works.resize(max_parrel);
        for (size_t i = 0, total = m_works.size(); i < total; i++) {
            Work* w = &m_works[i];
            rv = nng_aio_alloc(&w->aio, _serverCallback, w);
            CLS_CHECK(0 == rv, "Failed create work {}! {}", i, nng_strerror(rv));
            rv = nng_ctx_open(&w->ctx, m_socket);
            CLS_CHECK(0 == rv, "Failed open ctx {}! {}", i, nng_strerror(rv));
            w->state = Work::INIT;
            w->server = this;
        }

        for (size_t i = 0, total = m_works.size(); i < total; i++) {
            _serverCallback(&m_works[i]);
        }
    }

    void loop() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void stop() {
        HKU_IF_RETURN(m_works.empty(), void());
        for (size_t i = 0, total = m_works.size(); i < total; i++) {
            Work* w = &m_works[i];
            w->server = nullptr;
            w->state = Work::FINISH;
            if (w->aio) {
                nng_aio_stop(w->aio);
                nng_aio_free(w->aio);
                nng_ctx_close(w->ctx);
                w->aio = nullptr;
            }
        }

        // 关闭 socket 服务节点
        nng_listener_close(m_listener);
        nng_close(m_socket);
        m_works.clear();
        CLS_INFO("stopped node server.");
    }

private:
    struct Work {
        enum { INIT, RECV, SEND, FINISH } state = INIT;
        nng_aio* aio{nullptr};
        nng_ctx ctx;
        NodeServer* server{nullptr};
    };

    static void _serverCallback(void* arg) {
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
                    CLS_FATAL("Failed nng_ctx_send! {}", nng_strerror(rv));
                    work->state = Work::FINISH;
                    return;
                }
                work->state = Work::RECV;
                nng_ctx_recv(work->ctx, work->aio);
                break;

            case Work::FINISH:
                break;

            default:
                CLS_FATAL("nng bad state!");
                break;
        }
    }

    static void _processRequest(Work* work) {
        NodeServer* server = work->server;
        CLS_IF_RETURN(!server || !work->aio, void());
        nng_msg* msg = nullptr;
        json res;

        try {
            int rv = nng_aio_result(work->aio);
            HKU_CHECK(rv == 0, "Failed nng_aio_result!");

            msg = nng_aio_get_msg(work->aio);
            json req = decodeMsg(msg);
            NODE_CHECK(req.contains("cmd"), NodeErrorCode::MISSING_CMD, "Missing command!");

            // 兼容老版本数字cmd
            std::string cmd = req["cmd"].is_number() ? fmt::format("{}", req["cmd"].get<int>())
                                                     : req["cmd"].get<std::string>();
            auto iter = server->m_handles.find(cmd);
            NODE_CHECK(iter != server->m_handles.end(), NodeErrorCode::INVALID_CMD,
                       "The server does not know how to process the message: {}", cmd);

            // tcp 连接尝试获取客户端地址和端口加入 req 中
            req["remote_host"] = "";
            req["remote_port"] = 0;
            nng_pipe p = nng_msg_get_pipe(msg);
            if (nng_pipe_id(p) > 0) {
                uint16_t port = 0;
                nng_sockaddr ra;
                rv = nng_pipe_get_addr(p, NNG_OPT_REMADDR, &ra);
                if (rv == 0) {
                    if (ra.s_family == NNG_AF_INET) {
                        char ipAddr[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, (void*)&ra.s_in.sa_addr, ipAddr, INET_ADDRSTRLEN);
                        port = ntohs(ra.s_in.sa_port);
                        req["remote_host"] = ipAddr;
                        req["remote_port"] = port;
                    } else if (ra.s_family == NNG_AF_INET6) {
                        char ipAddr[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, (void*)&ra.s_in.sa_addr, ipAddr, INET6_ADDRSTRLEN);
                        port = ntohs(ra.s_in6.sa_port);
                        req["remote_host"] = ipAddr;
                        req["remote_port"] = port;
                    }
                }
            }

            res = iter->second(std::move(req));
            res["ret"] = NodeErrorCode::SUCCESS;
            encodeMsg(msg, res);

            nng_aio_set_msg(work->aio, msg);
            work->state = Work::SEND;
            nng_ctx_send(work->ctx, work->aio);

        } catch (const NodeNngError& e) {
            CLS_FATAL("{}", e.what());
            work->state = Work::FINISH;

        } catch (const NodeError& e) {
            CLS_ERROR("{}", e.what());
            res["ret"] = e.errcode();
            res["msg"] = e.what();
            encodeMsg(msg, res);
            nng_aio_set_msg(work->aio, msg);
            work->state = Work::SEND;
            nng_ctx_send(work->ctx, work->aio);

        } catch (const std::exception& e) {
            CLS_ERROR("{}", e.what());
            res["ret"] = NodeErrorCode::UNKNOWN_ERROR;
            res["msg"] = e.what();
            encodeMsg(msg, res);
            nng_aio_set_msg(work->aio, msg);
            work->state = Work::SEND;
            nng_ctx_send(work->ctx, work->aio);

        } catch (...) {
            std::string errmsg = "Unknown error!";
            CLS_ERROR("{}", errmsg);
            res["ret"] = NodeErrorCode::UNKNOWN_ERROR;
            res["msg"] = errmsg;
            nng_aio_set_msg(work->aio, msg);
            work->state = Work::SEND;
            nng_ctx_send(work->ctx, work->aio);
        }
    }

private:
    std::string m_addr;
    nng_socket m_socket;
    nng_listener m_listener;
    std::vector<Work> m_works;
    std::unordered_map<std::string, std::function<json(json&& req)>> m_handles;
};

}  // namespace hku