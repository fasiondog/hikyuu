/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <unordered_set>
#include <hikyuu/utilities/thread/ThreadPool.h>
#include "HttpMicro.h"

namespace hku {

class HttpServer {
public:
    HttpServer(const char *host, uint16_t port);
    virtual ~HttpServer();

    void start();

    void GET(const char *path, void (*rest_handle)(nng_aio *)) {
        regHandle("GET", path, rest_handle);
    }

    void POST(const char *path, void (*rest_handle)(nng_aio *)) {
        regHandle("POST", path, rest_handle);
    }

    void regHandle(const char *method, const char *path, void (*rest_handle)(nng_aio *));

private:
    static void signal_handler(int signal);

private:
    std::string m_root_url;
    std::string m_host;
    uint16_t m_port;

private:
    static nng_http_server *ms_server;
    static ThreadPool ms_tg;
};

}  // namespace hku