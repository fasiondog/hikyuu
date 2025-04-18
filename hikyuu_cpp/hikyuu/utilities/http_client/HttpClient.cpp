/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-07-26
 *      Author: fasiondog
 */

#include "HttpClient.h"

#if HKU_ENABLE_HTTP_CLIENT_ZIP
#include "gzip/compress.hpp"
#include "gzip/decompress.hpp"
#endif

#include <sstream>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/os.h"
#include "url.h"

namespace hku {

HttpResponse::HttpResponse() {
    NNG_CHECK(nng_http_res_alloc(&m_res));
}

HttpResponse::~HttpResponse() {
    if (m_res) {
        nng_http_res_free(m_res);
    }
}

void HttpResponse::reset() {
    if (m_res) {
        nng_http_res_free(m_res);
        NNG_CHECK(nng_http_res_alloc(&m_res));
    }
    m_body.clear();
}

HttpResponse::HttpResponse(HttpResponse&& rhs) : m_res(rhs.m_res), m_body(std::move(rhs.m_body)) {
    rhs.m_res = nullptr;
}

HttpResponse& HttpResponse::operator=(HttpResponse&& rhs) {
    if (this != &rhs) {
        if (m_res != nullptr) {
            nng_http_res_free(m_res);
        }
        m_res = rhs.m_res;
        rhs.m_res = nullptr;
        m_body = std::move(rhs.m_body);
    }
    return *this;
}

HttpClient::~HttpClient() {
    reset();
#if HKU_ENABLE_HTTP_CLIENT_SSL
    m_tls_cfg.release();
#endif
}

void HttpClient::reset() {
    m_client.release();
    m_conn.close();
    m_aio.release();
}

void HttpClient::setCaFile(const std::string& filename) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
    HKU_CHECK(!filename.empty(), "Input filename is empty!");
    HKU_IF_RETURN(filename == m_ca_file, void());
    HKU_CHECK(existFile(filename), "Not exist file: {}", filename);
    m_tls_cfg.set_ca_file(filename);
    m_ca_file = filename;
    reset();
#else
    HKU_THROW("Not support https! Please complie with --http_client_ssl!");
#endif
}

void HttpClient::_connect() {
    HKU_CHECK(m_url.valid(), "Invalid url: {}", m_url.raw_url());

    m_client.set_url(m_url);

    if (m_url.is_https()) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
        auto* old_cfg = m_client.get_tls_cfg();
        if (!old_cfg || old_cfg != m_tls_cfg.get()) {
            m_client.set_tls_cfg(m_tls_cfg.get());
        }
#endif
    }

    m_aio.alloc(m_timeout_ms);
    m_client.connect(m_aio);

    if (!m_conn.valid()) {
        NNG_CHECK(m_aio.wait().result());
        m_conn = nng::http_conn((nng_http_conn*)m_aio.get_output(0));
    }
}

HttpResponse HttpClient::request(const std::string& method, const std::string& path,
                                 const HttpParams& params, const HttpHeaders& headers,
                                 const char* body, size_t body_len,
                                 const std::string& content_type) {
    HKU_CHECK(m_url.valid(), "Invalid url: {}", m_url.raw_url());

    HttpResponse res;
    try {
        std::ostringstream buf;
        bool first = true;
        for (auto iter = params.cbegin(); iter != params.cend(); ++iter) {
            if (first) {
                buf << "?";
                first = false;
            } else {
                buf << "&";
            }
            buf << iter->first << "=" << iter->second;
        }

        std::string uri = buf.str();
        uri = uri.empty() ? path : fmt::format("{}{}", path, uri);
        res = _readResChunk(method, uri, headers, body, body_len, content_type);

        if (res.getHeader("Connection") == "close") {
            // HKU_TRACE("Connect closed");
            reset();
        }

    } catch (const std::exception&) {
        reset();
        throw;
    } catch (...) {
        reset();
        HKU_THROW_UNKNOWN;
    }
    return res;
}

HttpResponse HttpClient::_readResChunk(const std::string& method, const std::string& uri,
                                       const HttpHeaders& headers, const char* body,
                                       size_t body_len, const std::string& content_type) {
    HttpResponse res;
    nng::http_req req(m_url);
    req.set_method(method).set_uri(uri).add_headers(m_default_headers).add_headers(headers);
    if (body != nullptr) {
        HKU_CHECK(body_len > 0, "Body is not null, but len is zero!");
        req.add_header("Content-Type", content_type);

#if HKU_ENABLE_HTTP_CLIENT_ZIP
        if (req.get_header("Content-Encoding") == "gzip") {
            gzip::Compressor comp(Z_DEFAULT_COMPRESSION);
            std::string output;
            comp.compress(output, body, body_len);
            req.copy_data(output.data(), output.size());
        } else {
            req.set_data(body, body_len);
        }
#else
        req.del_header("Content-Encoding").set_data(body, body_len);
#endif
    }

    int count = 0;
    while (count < 2) {
        count++;
        _connect();

        m_conn.transact(req.get(), res.get(), m_aio);
        int rv = m_aio.wait().result();
        if (0 == rv) {
            break;
        } else if (NNG_ETIMEDOUT == rv) {
            throw HttpTimeoutException();
        } else if (NNG_ECLOSED == rv || NNG_ECONNSHUT == rv || NNG_ECONNREFUSED == rv) {
            // HKU_DEBUG("rv: {}", nng_strerror(rv));
            reset();
            res.reset();
        } else {
            HKU_THROW("[NNG_ERROR] {} ", nng_strerror(rv));
        }
    }

    void* data;
    size_t len;
    nng_http_res_get_data(res.get(), &data, &len);

    if (len > 0) {
#if HKU_ENABLE_HTTP_CLIENT_ZIP
        if (res.getHeader("Content-Encoding") == "gzip") {
            res.m_body = gzip::decompress((const char*)data, len);
        } else {
            res._resizeBody(len);
            memcpy(res.m_body.data(), data, len);
        }
#else
        res._resizeBody(len);
        memcpy(res.m_body.data(), data, len);
#endif
    }

    return res;
}

}  // namespace hku