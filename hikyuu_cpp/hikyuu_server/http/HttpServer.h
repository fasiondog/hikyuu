/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <unordered_set>
#include <hikyuu/utilities/thread/MQThreadPool.h>
#include <hikyuu/utilities/thread/ThreadPool.h>
#include <hikyuu/utilities/thread/FuncWrapper.h>
#include "HttpHandle.h"

namespace hku {

class HttpServer {
    CLASS_LOGGER(HttpServer)

public:
    HttpServer(const char *host, uint16_t port);
    virtual ~HttpServer();

    void start();

    template <typename Handle>
    void GET(const char *path) {
        regHandle("GET", path, [](nng_aio *aio) { ms_tg.submit(Handle(aio)); });
    }

    template <typename Handle>
    void POST(const char *path) {
        regHandle("POST", path, [](nng_aio *aio) { ms_tg.submit(Handle(aio)); });
    }

    template <typename Handle>
    void regHandle(const char *method, const char *path) {
        regHandle(method, path, [](nng_aio *aio) { ms_tg.submit(Handle(aio)); });
    }

private:
    void regHandle(const char *method, const char *path, void (*rest_handle)(nng_aio *));

private:
    std::string m_root_url;
    std::string m_host;
    uint16_t m_port;

private:
    static void http_exit();
    static void signal_handler(int signal);

private:
    static nng_http_server *ms_server;
    static MQThreadPool ms_tg;
};

}  // namespace hku