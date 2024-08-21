/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-07-26
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_HTTP_CLIENT_H
#define HKU_UTILS_HTTP_CLIENT_H

#include "hikyuu/utilities/config.h"
#if !HKU_ENABLE_HTTP_CLIENT
#error "Don't enable http client, please config with --http_client=y"
#endif

#include <string>
#include <nlohmann/json.hpp>
#include "nng_wrap.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

using json = nlohmann::json;

class HKU_UTILS_API HttpClient;

class HKU_UTILS_API HttpResponse final {
    friend class HKU_UTILS_API HttpClient;

public:
    HttpResponse();
    ~HttpResponse();

    HttpResponse(const HttpResponse&) = delete;
    HttpResponse& operator=(const HttpResponse&) = delete;

    HttpResponse(HttpResponse&& rhs);
    HttpResponse& operator=(HttpResponse&& rhs);

    const std::string& body() const noexcept {
        return m_body;
    }

    hku::json json() const {
        return json::parse(m_body);
    }

    int status() const noexcept {
        return nng_http_res_get_status(m_res);
    }

    std::string reason() noexcept {
        return nng_http_res_get_reason(m_res);
    }

    std::string getHeader(const std::string& key) noexcept {
        const char* hdr = nng_http_res_get_header(m_res, key.c_str());
        return hdr ? std::string(hdr) : std::string();
    }

    size_t getContentLength() noexcept {
        std::string slen = getHeader("Content-Length");
        return slen.empty() ? 0 : std::stoi(slen);
    }

private:
    void _resizeBody(size_t len) {
        m_body.resize(len);
    }

    nng_http_res* get() const noexcept {
        return m_res;
    }

    void reset();

private:
    nng_http_res* m_res{nullptr};
    std::string m_body;
};

class HKU_UTILS_API HttpClient {
public:
    HttpClient() = default;
    explicit HttpClient(const std::string& url, int32_t timeout_ms = NNG_DURATION_DEFAULT)
    : m_url(nng::url(url)), m_timeout_ms(timeout_ms) {};
    virtual ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpClient(HttpClient&& rhs)
    : m_default_headers(std::move(rhs.m_default_headers)),
      m_url(std::move(rhs.m_url)),
      m_client(std::move(rhs.m_client)),
      m_aio(std::move(rhs.m_aio)),
      m_conn(std::move(rhs.m_conn)),
#if HKU_ENABLE_HTTP_CLIENT_SSL
      m_tls_cfg(std::move(rhs.m_tls_cfg)),
      m_ca_file(std::move(rhs.m_ca_file)),
#endif
      m_timeout_ms(rhs.m_timeout_ms) {
    }

    HttpClient& operator=(HttpClient&& rhs) {
        if (this != &rhs) {
            m_default_headers = std::move(rhs.m_default_headers);
            m_url = std::move(rhs.m_url);
            m_client = (std::move(rhs.m_client));
            m_aio = std::move(rhs.m_aio);
            m_conn = std::move(rhs.m_conn);
#if HKU_ENABLE_HTTP_CLIENT_SSL
            m_tls_cfg = std::move(rhs.m_tls_cfg);
            m_ca_file = std::move(rhs.m_ca_file);
#endif
            m_timeout_ms = rhs.m_timeout_ms;
            rhs.m_timeout_ms = NNG_DURATION_DEFAULT;
        }
        return *this;
    }

    bool valid() const noexcept {
        return m_url.valid();
    }

    const std::string& url() const noexcept {
        return m_url.raw_url();
    }

    void setUrl(const std::string& url) noexcept {
        m_url = nng::url(url);
        reset();
    }

    // #define NNG_DURATION_INFINITE (-1)
    // #define NNG_DURATION_DEFAULT (-2)
    // #define NNG_DURATION_ZERO (0)
    void setTimeout(int32_t ms) {
        if (m_timeout_ms != ms) {
            m_timeout_ms = ms;
            reset();
        }
    }

    void setDefaultHeaders(const HttpHeaders& headers) {
        m_default_headers = headers;
    }

    void setDefaultHeaders(HttpHeaders&& headers) {
        m_default_headers = std::move(headers);
    }

    void setCaFile(const std::string& filename);

    void reset();

    HttpResponse request(const std::string& method, const std::string& path,
                         const HttpParams& params, const HttpHeaders& headers, const char* body,
                         size_t body_len, const std::string& content_type);

    HttpResponse get(const std::string& path, const HttpHeaders& headers = HttpHeaders()) {
        return request("GET", path, HttpParams(), headers, nullptr, 0, "");
    }

    HttpResponse get(const std::string& path, const HttpParams& params,
                     const HttpHeaders& headers) {
        return request("GET", path, params, headers, nullptr, 0, "");
    }

    HttpResponse post(const std::string& path, const HttpParams& params, const HttpHeaders& headers,
                      const char* body, size_t len, const std::string& content_type) {
        return request("POST", path, params, headers, body, len, content_type);
    }

    HttpResponse post(const std::string& path, const HttpHeaders& headers, const char* body,
                      size_t len, const std::string& content_type) {
        return request("POST", path, HttpParams(), headers, body, len, content_type);
    }

    HttpResponse post(const std::string& path, const HttpParams& params, const HttpHeaders& headers,
                      const std::string& content, const std::string& content_type = "text/plaint") {
        return post(path, params, headers, content.data(), content.size(), content_type);
    }

    HttpResponse post(const std::string& path, const HttpHeaders& headers,
                      const std::string& content, const std::string& content_type = "text/plaint") {
        return post(path, HttpParams(), headers, content, content_type);
    }

    HttpResponse post(const std::string& path, const HttpParams& params, const HttpHeaders& headers,
                      const json& body) {
        return post(path, params, headers, body.dump(), "application/json");
    }

    HttpResponse post(const std::string& path, const HttpHeaders& headers, const json& body) {
        return post(path, HttpParams(), headers, body);
    }

    HttpResponse post(const std::string& path, const json& body) {
        return post(path, HttpHeaders(), body);
    }

private:
    void _connect();
    HttpResponse _readResChunk(const std::string& method, const std::string& uri,
                               const HttpHeaders& headers, const char* body, size_t body_len,
                               const std::string& content_type);

private:
    HttpHeaders m_default_headers;
    nng::url m_url;
    nng::http_client m_client;
    nng::aio m_aio;
    nng::http_conn m_conn;
#if HKU_ENABLE_HTTP_CLIENT_SSL
    nng::tls_config m_tls_cfg;
    std::string m_ca_file;
#endif

    int32_t m_timeout_ms{NNG_DURATION_DEFAULT};
};

}  // namespace hku

#endif