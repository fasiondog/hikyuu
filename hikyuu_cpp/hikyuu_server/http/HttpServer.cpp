/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#include <csignal>
#include "HttpServer.h"

namespace hku {

nng_http_server* HttpServer::ms_server = nullptr;
ThreadPool HttpServer::ms_tg(1, false);

void HttpServer::signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        if (ms_server) {
            HKU_INFO("release server");
            if (ms_server) {
                nng_http_server_release(ms_server);
                ms_server = nullptr;
            }
        }
        ms_tg.stop();
        ms_tg.join();
        exit(0);
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