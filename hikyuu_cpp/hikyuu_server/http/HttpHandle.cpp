/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#include <string_view>
#include <hikyuu/utilities/arithmetic.h>
#include "url.h"
#include "HttpHandle.h"

namespace hku {

HttpHandle::HttpHandle(nng_aio* aio) : m_http_aio(aio) {}

void HttpHandle::operator()() {
    CLS_FATAL_IF_RETURN(!m_http_aio, void(), "http aio is null!");
    int rv = nng_http_res_alloc(&m_nng_res);
    if (rv != 0) {
        CLS_FATAL("Failed nng_http_res_alloc! {}", nng_strerror(rv));
        nng_aio_finish(m_http_aio, 0);
        return;
    }

    try {
        m_nng_req = (nng_http_req*)nng_aio_get_input(m_http_aio, 0);
        m_nng_conn = (nng_http_conn*)nng_aio_get_input(m_http_aio, 2);

        for (auto& filter : m_filters) {
            filter(this);
        }

        before_run();
        run();
        after_run();

        // nng_http_res_set_status(m_nng_res, NNG_HTTP_STATUS_OK);
        nng_aio_set_output(m_http_aio, 0, m_nng_res);
        nng_aio_finish(m_http_aio, 0);

    } catch (nlohmann::json::exception& e) {
        CLS_WARN("HttpBadRequestError({}): {}", INVALID_JSON_REQUEST, e.what());
        processException(NNG_HTTP_STATUS_BAD_REQUEST, INVALID_JSON_REQUEST, e.what());

    } catch (HttpError& e) {
        CLS_WARN("{}({}): {}", e.name(), e.errcode(), e.what());
        processException(e.status(), e.errcode(), e.what());

    } catch (std::exception& e) {
        CLS_ERROR("HttpError({}): {}", NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR, e.what());
        processException(NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                         NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR, e.what());

    } catch (...) {
        CLS_ERROR("HttpError({}): {}", NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR, "Unknown error");
        processException(NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                         NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR, "Unknown error");
    }

    if (ms_enable_trace) {
        trace();
    }
}

void HttpHandle::trace() {
    if (!ms_enable_only_traceid) {
        CLS_TRACE("{}", getTraceInfo());
        return;
    }

    std::string traceid = getReqHeader("traceid");
    if (!traceid.empty()) {
        CLS_TRACE("{}", getTraceInfo());
    }
}

std::string HttpHandle::getTraceInfo() {
    std::ostringstream out;
    const char* url = nng_http_req_get_uri(m_nng_req);
    if (url) {
        out << url << std::endl;
        out << "    url: " << url << std::endl;
    } else {
        out << "url: null" << std::endl;
        out << "    url: null" << std::endl;
    }

    std::string traceid = getReqHeader("traceid");
    if (!traceid.empty()) {
        out << "    traceid: " << traceid << std::endl;
    }

    char* data;
    size_t len = 0;
    getReqData((void**)&data, &len);
    out << "    request: ";
    if (data) {
        out << std::string_view(data, len) << std::endl;
    } else {
        out << "null" << std::endl;
    }

    out << "    response: ";
    nng_http_res_get_data(m_nng_res, (void**)&data, &len);
    if (data) {
        out << data << std::endl;
    } else {
        out << "null" << std::endl;
    }

    return out.str();
}

void HttpHandle::processException(int http_status, int errcode, std::string_view err_msg) {
    try {
        nng_http_res_set_header(m_nng_res, "Content-Type", "application/json; charset=UTF-8");
        nng_http_res_set_status(m_nng_res, http_status);
        nng_http_res_set_reason(m_nng_res, err_msg.data());
        setResData(
          fmt::format(R"({{"result":false,"errcode":{},"errmsg":"{}"}})", errcode, err_msg));
        nng_aio_set_output(m_http_aio, 0, m_nng_res);
        nng_aio_finish(m_http_aio, 0);
    } catch (std::exception& e) {
        CLS_ERROR(e.what());
        nng_aio_finish(m_http_aio, 0);
    } catch (...) {
        CLS_FATAL("unknown error in finished!");
        nng_aio_finish(m_http_aio, 0);
    }
}

std::string HttpHandle::getReqHeader(const char* name) const {
    std::string result;
    const char* head = nng_http_req_get_header(m_nng_req, name);
    if (head) {
        result = std::string(head);
    }
    return result;
}

std::string HttpHandle::getReqData() {
    void* data = nullptr;
    size_t len = 0;
    nng_http_req_get_data(m_nng_req, &data, &len);
    return data ? std::string((char*)data, len) : std::string();
}

json HttpHandle::getReqJson() {
    void* data;
    size_t len;
    nng_http_req_get_data(m_nng_req, &data, &len);
    json result;
    try {
        if (data) {
            result = json::parse(std::string_view((char*)data, len));
        }
    } catch (json::exception& e) {
        LOG_ERROR("Failed parse json: {}", (const char*)data);
        throw HttpBadRequestError(BadRequestErrorCode::INVALID_JSON_REQUEST, e.what());
    }
    return result;
}

bool HttpHandle::haveQueryParams() {
    const char* url = nng_http_req_get_uri(m_nng_req);
    return !url ? false : strchr(url, '?') != nullptr;
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

std::string HttpHandle::getLanguage() const {
    std::string lang = getReqHeader("Accept-Language");
    auto pos = lang.find_first_of(',');
    if (pos != std::string::npos) {
        lang = lang.substr(0, pos);
    }
    if (!lang.empty()) {
        to_lower(lang);
    }
    return lang;
}

}  // namespace hku