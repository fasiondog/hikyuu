/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-07-26
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_NNG_WRAP_H
#define HKU_UTILS_NNG_WRAP_H

#include <string>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>
#include "hikyuu/utilities/Log.h"

#include "hikyuu/utilities/config.h"
#if HKU_ENABLE_HTTP_CLIENT_SSL
#include <nng/supplemental/tls/tls.h>
#endif

namespace hku {

struct HttpTimeoutException : hku::exception {
    HttpTimeoutException() : hku::exception("Http timeout!") {}
    virtual ~HttpTimeoutException() noexcept = default;
};

using HttpHeaders = std::map<std::string, std::string>;
using HttpParams = std::map<std::string, std::string>;

}  // namespace hku

namespace hku {
namespace nng {

#ifndef NNG_CHECK
#define NNG_CHECK(rv)                                       \
    {                                                       \
        if (rv != 0) {                                      \
            HKU_THROW("[NNG_ERROR] {} ", nng_strerror(rv)); \
        }                                                   \
    }
#endif

#ifndef NNG_CHECK_M
#define NNG_CHECK_M(rv, ...)                                                              \
    {                                                                                     \
        if (rv != 0) {                                                                    \
            HKU_THROW("{} | [NNG_ERROR] {}", fmt::format(__VA_ARGS__), nng_strerror(rv)); \
        }                                                                                 \
    }
#endif

class url final {
public:
    url() = default;
    explicit url(const std::string& url_) noexcept : m_rawurl(url_) {
        nng_url_parse(&m_url, m_rawurl.c_str());
    }

    url(const url&) = delete;
    url(url&& rhs) noexcept : m_rawurl(std::move(rhs.m_rawurl)), m_url(rhs.m_url) {
        rhs.m_url = nullptr;
    }

    url& operator=(const url&) = delete;
    url& operator=(url&& rhs) noexcept {
        if (this != &rhs) {
            if (m_url != nullptr) {
                nng_url_free(m_url);
            }
            m_url = rhs.m_url;
            m_rawurl = std::move(rhs.m_rawurl);
            rhs.m_url = nullptr;
        }
        return *this;
    }

    ~url() {
        if (m_url) {
            nng_url_free(m_url);
        }
    }

    const std::string& raw_url() const noexcept {
        return m_rawurl;
    }

    nng_url* get() const noexcept {
        return m_url;
    }

    nng_url* operator->() const noexcept {
        return m_url;
    }

    bool valid() const noexcept {
        return m_url != nullptr;
    }

    bool is_https() const noexcept {
        return m_url == nullptr ? false : strcmp("https", m_url->u_scheme) == 0;
    }

private:
    std::string m_rawurl;
    nng_url* m_url{nullptr};
};

class aio final {
public:
    aio() = default;
    aio(const aio&) = delete;
    ~aio() {
        if (m_aio) {
            nng_aio_free(m_aio);
        }
    }

    aio(aio&& rhs) : m_aio(rhs.m_aio), m_timeout(rhs.m_timeout) {
        rhs.m_aio = nullptr;
        rhs.m_timeout = NNG_DURATION_DEFAULT;
    }

    aio& operator=(const aio&) = delete;

    aio& operator=(aio&& rhs) {
        if (this != &rhs) {
            m_aio = rhs.m_aio;
            m_timeout = rhs.m_timeout;
            rhs.m_aio = nullptr;
            rhs.m_timeout = NNG_DURATION_DEFAULT;
        }
        return *this;
    }

    bool valid() const noexcept {
        return m_aio != nullptr;
    }

    nng_aio* get() const noexcept {
        return m_aio;
    }

    nng_aio* operator->() const noexcept {
        return m_aio;
    }

    void alloc(int32_t timeout_ms) {
        if (m_aio == nullptr) {
            NNG_CHECK(nng_aio_alloc(&m_aio, NULL, NULL));
        }
        set_timeout(timeout_ms);
    }

    void release() {
        if (m_aio) {
            nng_aio_free(m_aio);
            m_aio = nullptr;
        }
    }

    aio& wait() {
        nng_aio_wait(m_aio);
        return *this;
    }

    int result() {
        // 直接返回结果，在调用处判断异常，才知道是在具体哪里
        return nng_aio_result(m_aio);
    }

    void* get_output(unsigned index) {
        return nng_aio_get_output(m_aio, index);
    }

    /*
     * 0 - 恢复默认值
     * <0 - 不限制
     */
    void set_timeout(int32_t ms) {
        if (ms != m_timeout) {
            m_timeout = ms;
            nng_aio_set_timeout(m_aio, ms);
        }
    }

    int32_t get_timeout() const noexcept {
        return m_timeout;
    }

    void set_iov(unsigned n, const nng_iov* iov) {
        NNG_CHECK(nng_aio_set_iov(m_aio, n, iov));
    }

private:
    nng_aio* m_aio{nullptr};
    int32_t m_timeout{NNG_DURATION_DEFAULT};
};

#if HKU_ENABLE_HTTP_CLIENT_SSL
class tls_config final {
public:
    tls_config() = default;

    tls_config(const tls_config& th) : m_cfg(th.m_cfg) {
        if (m_cfg) {
            nng_tls_config_hold(th.m_cfg);
        }
    }

    tls_config(tls_config&& rhs) : m_cfg(rhs.m_cfg) {
        rhs.m_cfg = nullptr;
    }

    ~tls_config() {
        if (m_cfg) {
            nng_tls_config_free(m_cfg);
        }
    }

    tls_config& operator=(const tls_config& th) {
        if (this != &th) {
            m_cfg = th.m_cfg;
            if (m_cfg) {
                nng_tls_config_hold(m_cfg);
            }
        }
        return *this;
    }

    tls_config& operator=(tls_config&& rhs) {
        if (this != &rhs) {
            m_cfg = rhs.m_cfg;
            rhs.m_cfg = nullptr;
        }
        return *this;
    }

    void release() {
        if (m_cfg) {
            nng_tls_config_free(m_cfg);
            m_cfg = nullptr;
        }
    }

    nng_tls_config* get() const noexcept {
        return m_cfg;
    }

    tls_config& set_ca_file(const std::string& filename) {
        NNG_CHECK(alloc());
        NNG_CHECK(nng_tls_config_ca_file(m_cfg, filename.c_str()));
        return *this;
    }

private:
    int alloc() {
        return m_cfg ? 0 : nng_tls_config_alloc(&m_cfg, NNG_TLS_MODE_CLIENT);
    }

private:
    nng_tls_config* m_cfg{nullptr};
};
#endif  // #if HKU_ENABLE_HTTP_CLIENT_SSL

class http_client final {
public:
    http_client() = default;
    ~http_client() {
        if (m_client) {
            nng_http_client_free(m_client);
        }
    }

    http_client(const http_client&) = delete;
    http_client& operator=(const http_client&) = delete;

    http_client(http_client&& rhs)
    : m_client(rhs.m_client), m_aio(rhs.m_aio), m_tls_cfg(rhs.m_tls_cfg) {
        rhs.m_client = nullptr;
        rhs.m_aio = nullptr;
        rhs.m_tls_cfg = nullptr;
    }

    http_client& operator=(http_client&& rhs) {
        if (this != &rhs) {
            m_client = rhs.m_client;
            m_aio = rhs.m_aio;
            m_tls_cfg = rhs.m_tls_cfg;
            rhs.m_client = nullptr;
            rhs.m_aio = nullptr;
            rhs.m_tls_cfg = nullptr;
        }
        return *this;
    }

    void set_url(const nng::url& url) {
#if !HKU_ENABLE_HTTP_CLIENT_SSL
        if (url.is_https()) {
            HKU_THROW("Not support https: ({})! Please compile with --http_client_ssl",
                      url.raw_url());
        }
#endif
        if (!m_client) {
            NNG_CHECK(nng_http_client_alloc(&m_client, url.get()));
            m_tls_cfg = nullptr;
            m_aio = nullptr;
        }
    }

    void connect(const aio& aio) {
        if (m_aio != aio.get()) {
            nng_http_client_connect(m_client, aio.get());
            m_aio = aio.get();
        }
    }

    void set_tls_cfg(nng_tls_config* cfg) {
        if (cfg != m_tls_cfg) {
            NNG_CHECK(nng_http_client_set_tls(m_client, cfg));
            m_tls_cfg = cfg;
        }
    }

    nng_tls_config* get_tls_cfg() const noexcept {
        return m_tls_cfg;
    }

    nng_http_client* get() const noexcept {
        return m_client;
    }

    nng_http_client* operator->() const noexcept {
        return m_client;
    }

    explicit operator bool() const noexcept {
        return m_client != nullptr;
    }

    void release() {
        if (m_client) {
            nng_http_client_free(m_client);
            m_client = nullptr;
            m_aio = nullptr;
            m_tls_cfg = nullptr;
        }
    }

private:
    nng_http_client* m_client{nullptr};
    nng_aio* m_aio{nullptr};
    nng_tls_config* m_tls_cfg{nullptr};
};

class http_req final {
public:
    http_req() = default;
    explicit http_req(const url& url) {
        NNG_CHECK(nng_http_req_alloc(&m_req, url.get()));
    }

    http_req(const http_req&) = delete;
    http_req(http_req&& rhs) : m_req(rhs.m_req) {
        rhs.m_req = nullptr;
    }

    ~http_req() {
        if (m_req) {
            nng_http_req_free(m_req);
        }
    }

    http_req& operator=(const http_req&) = delete;
    http_req& operator=(http_req&& rhs) {
        if (this != &rhs) {
            if (m_req) {
                nng_http_req_free(m_req);
            }
            m_req = rhs.m_req;
            rhs.m_req = nullptr;
        }
        return *this;
    }

    nng_http_req* get() const noexcept {
        return m_req;
    }

    http_req& set_method(const std::string& method) {
        NNG_CHECK(nng_http_req_set_method(m_req, method.c_str()));
        return *this;
    }

    http_req& set_uri(const std::string& uri) {
        NNG_CHECK(nng_http_req_set_uri(m_req, uri.c_str()));
        return *this;
    }

    http_req& add_header(const std::string& key, const std::string& val) {
        NNG_CHECK_M(nng_http_req_add_header(m_req, key.c_str(), val.c_str()),
                    "Failed add head {}: {}", key, val);
        return *this;
    }

    http_req& add_headers(const HttpHeaders& headers) {
        for (auto iter = headers.cbegin(); iter != headers.cend(); ++iter) {
            NNG_CHECK_M(nng_http_req_add_header(m_req, iter->first.c_str(), iter->second.c_str()),
                        "Failed add header {}: {}", iter->first, iter->second);
        }
        return *this;
    }

    std::string get_header(const std::string& key) {
        const char* head = nng_http_req_get_header(m_req, key.c_str());
        return head ? std::string(head) : std::string();
    }

    http_req& del_header(const std::string& key) {
        nng_http_req_del_header(m_req, key.c_str());
        return *this;
    }

    /* 注: data 需要自行管理且在 req 释放之前应该一直存在，主要避免拷贝 */
    http_req& set_data(const char* data, size_t len) {
        if (data != nullptr && len != 0) {
            NNG_CHECK(nng_http_req_set_data(m_req, data, len));
        }
        return *this;
    }

    http_req& copy_data(const char* data, size_t len) {
        if (data != nullptr && len != 0) {
            NNG_CHECK(nng_http_req_copy_data(m_req, data, len));
        }
        return *this;
    }

private:
    nng_http_req* m_req{nullptr};
};

class http_conn final {
public:
    http_conn() = default;
    explicit http_conn(nng_http_conn* conn_) noexcept : m_conn(conn_) {}

    http_conn(const http_conn&) = delete;

    http_conn(http_conn&& rhs) noexcept : m_conn(rhs.m_conn) {
        rhs.m_conn = nullptr;
    }

    http_conn& operator=(const http_conn& rhs) = delete;

    http_conn& operator=(http_conn&& rhs) noexcept {
        if (this != &rhs) {
            if (m_conn != nullptr) {
                nng_http_conn_close(m_conn);
            }
            m_conn = rhs.m_conn;
            rhs.m_conn = nullptr;
        }
        return *this;
    }

    ~http_conn() {
        if (m_conn) {
            nng_http_conn_close(m_conn);
        }
    }

    void close() {
        if (m_conn) {
            nng_http_conn_close(m_conn);
            m_conn = nullptr;
        }
    }

    nng_http_conn* get() const noexcept {
        return m_conn;
    }

    nng_http_conn* operator->() const noexcept {
        return m_conn;
    }

    bool valid() const noexcept {
        return m_conn != nullptr;
    }

    void write_req(const http_req& req, const aio& aio) {
        nng_http_conn_write_req(m_conn, req.get(), aio.get());
    }

    void read_res(nng_http_res* res, const aio& aio) {
        nng_http_conn_read_res(m_conn, res, aio.get());
    }

    void read_all(const aio& aio) {
        nng_http_conn_read_all(m_conn, aio.get());
    }

    void transact(nng_http_req* req, nng_http_res* res, const aio& aio) {
        nng_http_conn_transact(m_conn, req, res, aio.get());
    }

private:
    nng_http_conn* m_conn{nullptr};
};

}  // namespace nng
}  // namespace hku

#endif