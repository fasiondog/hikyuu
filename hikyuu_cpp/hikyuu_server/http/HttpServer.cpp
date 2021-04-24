/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#include <csignal>
#include "HttpServer.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace hku {

#define HTTP_FATAL_CHECK(rv, msg)                                        \
    {                                                                    \
        if (rv != 0) {                                                   \
            CLS_FATAL("[HTTP_FATAL] {} err: {}", msg, nng_strerror(rv)); \
            http_exit();                                                 \
        }                                                                \
    }

nng_http_server* HttpServer::ms_server = nullptr;
MQStealThreadPool HttpServer::ms_tg(std::thread::hardware_concurrency(), false);

#if defined(_WIN32)
static UINT g_old_cp;
#endif

void HttpServer::http_exit() {
    stop();
    exit(0);
}

void HttpServer::signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        http_exit();
    }
}

HttpServer::HttpServer(const char* host, uint16_t port) : m_host(host), m_port(port) {
    HKU_CHECK(ms_server == nullptr, "Can only one server!");
    m_root_url = fmt::format("{}:{}", m_host, m_port);
    nng_url* url = nullptr;
    HTTP_FATAL_CHECK(nng_url_parse(&url, m_root_url.c_str()), "Failed nng_url_parse!");
    HTTP_FATAL_CHECK(nng_http_server_hold(&ms_server, url), "Failed nng_http_server_hold!");
    nng_url_free(url);
}

HttpServer::~HttpServer() {}

void HttpServer::start() {
    std::signal(SIGINT, &HttpServer::signal_handler);
    std::signal(SIGTERM, &HttpServer::signal_handler);

    HTTP_FATAL_CHECK(nng_http_server_start(ms_server), "Failed nng_http_server_start!");

#if defined(_WIN32)
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto g_old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void HttpServer::stop() {
    ms_tg.stop();
    if (ms_server) {
        CLS_INFO("Quit Http server");
        nng_http_server_stop(ms_server);
        nng_http_server_release(ms_server);
        nng_fini();
        ms_server = nullptr;
#if defined(_WIN32)
        SetConsoleOutputCP(g_old_cp);
#endif
    }
}

void HttpServer::regHandle(const char* method, const char* path, void (*rest_handle)(nng_aio*)) {
    try {
        HKU_CHECK(strlen(path) > 1, "Invalid api path!");
        HKU_CHECK(path[0] == '/', "The api path must start with '/', but current is '{}'", path[0]);
    } catch (std::exception& e) {
        CLS_FATAL(e.what());
        http_exit();
    }
    nng_url* url = nullptr;
    nng_http_handler* handler = nullptr;
    HTTP_FATAL_CHECK(nng_url_parse(&url, fmt::format("{}/{}", m_root_url, path).c_str()),
                     "Failed nng_url_parse!");
    HTTP_FATAL_CHECK(nng_http_handler_alloc(&handler, url->u_path, rest_handle),
                     "Failed nng_http_handler_alloc!");
    HTTP_FATAL_CHECK(nng_http_handler_set_method(handler, method),
                     "Failed nng_http_handler_set_method!");
    HTTP_FATAL_CHECK(nng_http_server_add_handler(ms_server, handler),
                     "Failed nng_http_server_add_handler!");
}

}  // namespace hku