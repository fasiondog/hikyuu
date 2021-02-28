/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#include <csignal>
#include "HttpServer.h"

namespace hku {

#define HTTP_FATAL_CHECK(rv, msg)                                        \
    {                                                                    \
        if (rv != 0) {                                                   \
            HKU_FATAL("[HTTP_FATAL] {} err: {}", msg, nng_strerror(rv)); \
            http_exit();                                                 \
        }                                                                \
    }

nng_http_server* HttpServer::ms_server = nullptr;
ThreadPool HttpServer::ms_tg(2 * std::thread::hardware_concurrency(), false);

void HttpServer::http_exit() {
    HKU_INFO("waiting exit ...");
    ms_tg.stop();
    ms_tg.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (ms_server) {
        nng_http_server_release(ms_server);
        nng_fini();
        ms_server = nullptr;
    }
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

HttpServer::~HttpServer() {
    ms_tg.join();
    if (ms_server) {
        HKU_INFO("Quit Http server");
        nng_http_server_release(ms_server);
        ms_server = nullptr;
    }
}

void HttpServer::start() {
    std::signal(SIGINT, &HttpServer::signal_handler);
    std::signal(SIGTERM, &HttpServer::signal_handler);
    HTTP_FATAL_CHECK(nng_http_server_start(ms_server), "Failed nng_http_server_start!");
}

void HttpServer::regHandle(const char* method, const char* path, void (*rest_handle)(nng_aio*)) {
    try {
        HKU_CHECK(strlen(path) > 1, "Invalid api path!");
        HKU_CHECK(path[0] == '/', "The api path must start with '/', but current is '{}'", path[0]);
    } catch (std::exception& e) {
        HKU_FATAL(e.what());
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