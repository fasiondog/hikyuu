/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-04-25
 *      Author: fasiondog
 */

#pragma once

#include <atomic>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "hikyuu/datetime/Datetime.h"
#include "NodeMessage.h"

namespace hku {

class NodeClient {
public:
    NodeClient() = default;

    explicit NodeClient(const std::string& serverAddr) : m_server_addr(serverAddr), m_port(0) {}

    virtual ~NodeClient() {
        close();
    }

    /** 设置服务端地址 */
    void setServerAddr(const std::string& serverAddr) {
        m_server_addr = serverAddr;
    }

    /** 连接服务器 */
    bool dial() noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        close();
        // HKU_TRACE("dial: {}", m_server_addr);
        int rv = nng_req0_open(&m_socket);
        // HKU_ERROR_IF_RETURN(rv != 0, false, "Failed open req socket! {}", nng_strerror(rv));
        HKU_IF_RETURN(rv != 0, false);
        m_connected = true;

        try {
            // 设置发送结果 socket 连接参数
            rv = nng_setopt_ms(m_socket, NNG_OPT_RECONNMINT, 10);
            NODE_NNG_CHECK(rv, "Failed nng_setopt_ms!");

            rv = nng_setopt_ms(m_socket, NNG_OPT_RECONNMAXT, 15000);
            NODE_NNG_CHECK(rv, "Failed nng_setopt_ms!");

            rv = nng_setopt_ms(m_socket, NNG_OPT_SENDTIMEO, 10000);
            NODE_NNG_CHECK(rv, "Failed nng_setopt_ms!");

            rv = nng_setopt_ms(m_socket, NNG_OPT_RECVTIMEO, 10000);
            NODE_NNG_CHECK(rv, "Failed nng_setopt_ms!");

            rv = nng_dial(m_socket, m_server_addr.c_str(), NULL, 0);
            NODE_NNG_CHECK(rv, "Failed dial server: {}!", m_server_addr);

            return true;

        } catch (const std::exception& e) {
            // HKU_ERROR("Failed dail server: {}! {}", m_server_addr, e.what());
        } catch (...) {
            // HKU_ERROR("Failed dail server: {}! Unknown error!", m_server_addr);
        }

        m_connected = false;
        nng_close(m_socket);
        return false;
    }

    /** 关闭连接 */
    void close() noexcept {
        if (m_connected) {
            nng_close(m_socket);
            m_connected = false;
        }
    }

    /** 当前连接状态 */
    bool connected() const {
        return m_connected;
    }

    /** 获取最后一次接收到服务端响应的时间 */
    Datetime getLastAckTime() const {
        return m_last_ack_time;
    }

    std::string host() const {
        return m_host;
    }

    uint16_t port() const {
        return m_port;
    }

    /**
     * 发送消息
     * @tparam sendT 发送消息体类型
     * @tparam recvT 响应消息体类型
     * @param sendType 发送消息类型
     * @param sendmsg 待发送消息体
     * @param recvType 响应消息类型
     * @param recvmsg[out] 输出响应消息体
     */
    bool post(json& req, json& res) noexcept {
        // 保证和服务器的通信必须是 req/res 模式
        std::lock_guard<std::mutex> lock(m_mutex);
        return _send(req) && _recv(res);
    }

private:
    bool _send(json& req) const noexcept {
        bool success = false;
        // HKU_ERROR_IF_RETURN(!m_connected, success, "Not connected!");
        HKU_IF_RETURN(!m_connected, success);

        nng_msg* msg = nullptr;
        int rv = nng_msg_alloc(&msg, 0);
        // HKU_ERROR_IF_RETURN(rv != 0, success, "Failed nng_msg_alloc! {}", nng_strerror(rv));
        HKU_IF_RETURN(rv != 0, success);

        try {
            encodeMsg(msg, req);
            rv = nng_sendmsg(m_socket, msg, 0);
            NODE_NNG_CHECK(rv, "Failed nng_sendmsg!");
            success = true;

        } catch (const std::exception& e) {
            // HKU_ERROR("Failed send result! {}", e.what());
        } catch (...) {
            // HKU_ERROR("Failed send result! Unknown error!");
        }

        if (!success) {
            nng_msg_free(msg);
        }

        return success;
    }

    bool _recv(json& res) noexcept {
        bool success = false;
        nng_msg* msg{nullptr};
        int rv = nng_recvmsg(m_socket, &msg, 0);
        // HKU_ERROR_IF_RETURN(rv != 0, success, "Failed nng_recvmsg! {}", nng_strerror(rv));
        HKU_IF_RETURN(rv != 0, success);
        m_last_ack_time = Datetime::now();

        try {
            res = decodeMsg(msg);
            success = true;

            nng_pipe p = nng_msg_get_pipe(msg);
            if (nng_pipe_id(p) > 0) {
                nng_sockaddr ra;
                rv = nng_pipe_get_addr(p, NNG_OPT_LOCADDR, &ra);
                if (rv == 0) {
                    if (ra.s_family == NNG_AF_INET) {
                        char ipAddr[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, (void*)&ra.s_in.sa_addr, ipAddr, INET_ADDRSTRLEN);
                        m_port = ntohs(ra.s_in.sa_port);
                        m_host = std::string(ipAddr);
                    } else if (ra.s_family == NNG_AF_INET6) {
                        char ipAddr[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, (void*)&ra.s_in.sa_addr, ipAddr, INET6_ADDRSTRLEN);
                        m_port = ntohs(ra.s_in6.sa_port);
                        m_host = std::string(ipAddr);
                    }
                }
            }

        } catch (const std::exception& e) {
            // HKU_ERROR("Failed recv response! {}", e.what());
        } catch (...) {
            // HKU_ERROR("Failed recv response! Unknown error!");
        }

        nng_msg_free(msg);
        return success;
    }

private:
    std::mutex m_mutex;
    std::string m_server_addr;  // 服务端地址
    nng_socket m_socket;
    std::atomic_bool m_connected{false};
    Datetime m_last_ack_time{Datetime::now()};  // 最后一次接收服务端响应的时间
    std::string m_host;
    uint16_t m_port;
};

}  // namespace hku