/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include "HttpMicro.h"

namespace hku {

class HttpServer {
public:
    HttpServer(const char *host, uint16_t port);
    virtual ~HttpServer();

    void start();

private:
    std::string m_host;
    uint16_t m_port;
    std::string m_root_url;

    nng_http_server *m_server = nullptr;
};

}  // namespace hku