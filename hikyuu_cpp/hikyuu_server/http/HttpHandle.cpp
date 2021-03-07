/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "url.h"
#include "HttpHandle.h"

namespace hku {

HttpHandle::HttpHandle(nng_aio* aio) : m_http_aio(aio) {}

void HttpHandle::operator()() {
    CLS_FATAL_IF_RETURN(!m_http_aio, void(), "http aio is null!");
    int rv = nng_http_res_alloc(&m_nng_res);
    if (rv != 0) {
        CLS_FATAL("Failed nng_http_res_alloc! {}", nng_strerror(rv));
        return;
    }

    try {
        m_nng_req = (nng_http_req*)nng_aio_get_input(m_http_aio, 0);
        m_nng_conn = (nng_http_conn*)nng_aio_get_input(m_http_aio, 2);

        before_run();
        run();
        // after_run();

        nng_aio_set_output(m_http_aio, 0, m_nng_res);
        nng_aio_finish(m_http_aio, 0);

    } catch (HttpHandleRunException& e) {
        error(e.errcode());
        nng_aio_set_output(m_http_aio, 0, m_nng_res);
        nng_aio_finish(m_http_aio, 0);

    } catch (std::exception& e) {
        std::string errmsg(e.what());
        CLS_ERROR(errmsg);
        unknown_error(errmsg);

    } catch (...) {
        std::string errmsg("Unknown error!");
        CLS_ERROR(errmsg);
        unknown_error(errmsg);
    }
}

void HttpHandle::unknown_error(const std::string& errmsg) {
    try {
        int errcode = NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR;
        const char* info = "Internal server error!";
        std::string html_template(
          R"(<!DOCTYPE html>
        <html><head><title>{} {}</title>
        <style>"
        body {{ font-family: Arial, sans serif; text-align: center }}
        h1 {{ font-size: 36px; }}
        span {{ background-color: gray; color: white; padding: 7px; border-radius: 5px }}
        h2 {{ font-size: 24px; }}
        p {{ font-size: 20px; }}
        </style></head>
        <body><p>&nbsp;</p>
        <h1><span>{}</span></h1>
        <h2>{}</h2>
        <p>{}</p>
        </body></html>)");
        std::string html = fmt::format(html_template, errcode, info, errcode, info, errmsg);
        nng_http_res_set_status(m_nng_res, errcode);
        nng_http_res_set_reason(m_nng_res, errmsg.c_str());
        nng_http_res_set_header(m_nng_res, "Content-Type", "text/html; charset=UTF-8");
        nng_http_res_copy_data(m_nng_res, html.c_str(), html.size());
        nng_aio_set_output(m_http_aio, 0, m_nng_res);
        nng_aio_finish(m_http_aio, 0);
    } catch (...) {
        CLS_FATAL("unknown error in finished!");
    }
}

std::string HttpHandle::getReqData() {
    void* data = nullptr;
    size_t len = 0;
    nng_http_req_get_data(m_nng_req, &data, &len);
    return data ? std::string((char*)data) : std::string();
}

bool HttpHandle::haveQueryParams() {
    const char* url = nng_http_req_get_uri(m_nng_req);
    return !url ? false : strchr(url, '?');
}

bool HttpHandle::getQueryParams(QueryParams& query_params) {
    const char* url = nng_http_req_get_uri(m_nng_req);
    CLS_IF_RETURN(!url, false);

    const char* p = strchr(url, '?');
    CLS_IF_RETURN(!p, false);

    p = p + 1;

    enum {
        s_key,
        s_value,
    } state = s_key;

    const char* key = p;
    const char* value = NULL;
    int key_len = 0;
    int value_len = 0;
    while (*p != '\0') {
        if (*p == '&') {
            if (key_len && value_len) {
                std::string strkey = std::string(key, key_len);
                std::string strvalue = std::string(value, value_len);
                query_params[url_unescape(strkey.c_str())] = url_unescape(strvalue.c_str());
                key_len = value_len = 0;
            }
            state = s_key;
            key = p + 1;
        } else if (*p == '=') {
            state = s_value;
            value = p + 1;
        } else {
            state == s_key ? ++key_len : ++value_len;
        }
        ++p;
    }
    if (key_len && value_len) {
        std::string strkey = std::string(key, key_len);
        std::string strvalue = std::string(value, value_len);
        query_params[url_unescape(strkey.c_str())] = url_unescape(strvalue.c_str());
        key_len = value_len = 0;
    }

    return query_params.size() != 0;
}

}  // namespace hku