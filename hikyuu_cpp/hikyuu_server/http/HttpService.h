/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "HttpServer.h"

namespace hku {

class HttpService {
public:
    HttpService() : HttpService("") {}
    HttpService(const char *url) : m_url(url) {}

    virtual void regHandle() = 0;

    void bind(HttpServer *server) {
        if (!server) {
            throw std::invalid_argument("Invalid server(nullptr)!");
        }
        m_server = server;
        regHandle();
    }

    template <typename Handle>
    void GET(const char *path) {
        std::string url(fmt::format("/{}/{}", m_url, path));
        m_server->GET<Handle>(url.c_str());
    }

    template <typename Handle>
    void POST(const char *path) {
        std::string url(fmt::format("/{}/{}", m_url, path));
        m_server->POST<Handle>(url.c_str());
    }

    template <typename Handle>
    void PUT(const char *path) {
        std::string url(fmt::format("/{}/{}", m_url, path));
        m_server->PUT<Handle>(url.c_str());
    }

    template <typename Handle>
    void DEL(const char *path) {
        std::string url(fmt::format("/{}/{}", m_url, path));
        m_server->DEL<Handle>(url.c_str());
    }

    template <typename Handle>
    void PATCH(const char *path) {
        std::string url(fmt::format("/{}/{}", m_url, path));
        m_server->PATCH<Handle>(url.c_str());
    }

private:
    std::string m_url;
    HttpServer *m_server{nullptr};
};

#define HTTP_SERVICE_IMP(cls) \
public:                       \
    cls() : HttpService() {}  \
    cls(const char *url) : HttpService(url) {}

}  // namespace hku