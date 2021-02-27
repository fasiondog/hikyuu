/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#include "HttpServer.h"

namespace hku {

HttpServer::HttpServer(const char* host, uint16_t port)
: m_host(host), m_port(port), m_server(nullptr) {
    m_root_url = fmt::format("{}:{}", m_host, m_port);
    nng_url* url{nullptr};
    HTTP_FATAL_CHECK(nng_url_parse(&url, m_root_url.c_str()), "Failed nng_url_parse!");
    HTTP_FATAL_CHECK(nng_http_server_hold(&m_server, url), "Failed nng_http_server_hold!");
    nng_url_free(url);
}

HttpServer::~HttpServer() {
    HKU_INFO("Quit http server({})", m_root_url);
    nng_http_server_release(m_server);
}

void HttpServer::start() {
    HTTP_FATAL_CHECK(nng_http_server_start(m_server), "Failed nng_http_server_start!");
}

}  // namespace hku