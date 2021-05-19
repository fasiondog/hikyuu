/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string_view>
#include <vector>
#include <functional>

#include <nlohmann/json.hpp>
#include "HttpError.h"
#include "common/mo.h"
#include "common/log.h"

using json = nlohmann::json;                  // 不保持插入排序
using ordered_json = nlohmann::ordered_json;  // 保持插入排序

namespace hku {

// 仅内部使用
#define NNG_CHECK(rv)                                      \
    {                                                      \
        if (rv != 0) {                                     \
            LOG_THROW("[NNG_ERROR] {}", nng_strerror(rv)); \
        }                                                  \
    }

// 仅内部使用
#define NNG_CHECK_M(rv, msg)                                             \
    {                                                                    \
        if (rv != 0) {                                                   \
            LOG_THROW("[HTTP_ERROR] {} err: {}", msg, nng_strerror(rv)); \
        }                                                                \
    }

class HttpHandle {
    CLASS_LOGGER(HttpHandle)

public:
    HttpHandle() = delete;
    HttpHandle(nng_aio *aio);
    virtual ~HttpHandle() {}

    /** 前处理 */
    virtual void before_run() {}

    /** 响应处理 */
    virtual void run() = 0;

    /** 后处理 */
    virtual void after_run() {}

    void addFilter(std::function<void(HttpHandle *)> filter) {
        m_filters.push_back(filter);
    }

    /**
     * 获取请求头部信息
     * @param name 头部信息名称
     * @return 如果获取不到将返回""
     */
    std::string getReqHeader(const char *name) const;

    /**
     * 获取请求头部信息
     * @param name 头部信息名称
     * @return 如果获取不到将返回""
     */
    std::string getReqHeader(const std::string &name) const {
        return getReqHeader(name.c_str());
    }

    /**
     * 获取请求数据
     * @param[out] data 请求中的数据起始地址，无数据时返回 NULL
     * @param[out] len 请求中的数据长度
     * @note 请求中无数据时, len返回的长度可能不为0
     */
    void getReqData(void **data, size_t *len) {
        nng_http_req_get_data(m_nng_req, data, len);
    }

    std::string getReqData();

    json getReqJson();

    /**
     * 请求的 ulr 中是否包含 query 参数
     */
    bool haveQueryParams();

    typedef std::unordered_map<std::string, std::string> QueryParams;

    /**
     * 获取 query 参数
     * @param query_params [out] 输出 query 参数
     * @return true | false 获取或解析失败
     */
    bool getQueryParams(QueryParams &query_params);

    void setResStatus(uint16_t status) {
        NNG_CHECK(nng_http_res_set_status(m_nng_res, status));
    }

    void setResHeader(const char *key, const char *val) {
        NNG_CHECK(nng_http_res_set_header(m_nng_res, key, val));
    }

    void setResData(const char *data) {
        NNG_CHECK(nng_http_res_copy_data(m_nng_res, data, strlen(data)));
    }

    void setResData(const std::string &content) {
        NNG_CHECK(nng_http_res_copy_data(m_nng_res, content.data(), content.size()));
    }

    void setResData(const json &data) {
        setResData(data.dump());
    }

    void setResData(const ordered_json &data) {
        setResData(data.dump());
    }

    /**
     * 从 Accept-Language 获取第一个语言类型
     * @note 非严格 html 协议，仅返回排在最前面的语言类型
     */
    std::string getLanguage() const;

    /**
     * 多语言翻译
     * @param msgid 待翻译的字符串
     */
    std::string _tr(const char *msgid) const {
        return MOHelper::translate(getLanguage(), msgid);
    }

    /**
     * 多语言翻译
     * @param ctx 翻译上下文
     * @param msgid 待翻译的字符串
     */
    std::string _ctr(const char *ctx, const char *msgid) {
        return MOHelper::translate(getLanguage(), ctx, msgid);
    }

    void operator()();

private:
    // error未捕获的信息，统一返回500页面
    void unknown_error(const std::string &errmsg);

    template <typename Error>
    void processHttpError(const Error &e);

    void processJsonError(const nlohmann::json::exception &e);

protected:
    nng_aio *m_http_aio{nullptr};
    nng_http_res *m_nng_res{nullptr};
    nng_http_req *m_nng_req{nullptr};
    nng_http_conn *m_nng_conn{nullptr};
    std::vector<std::function<void(HttpHandle *)>> m_filters;
};

#define HTTP_HANDLE_IMP(cls) \
public:                      \
    cls(nng_aio *aio) : HttpHandle(aio) {}

template <class Error>
void HttpHandle::processHttpError(const Error &e) {
    CLS_TRACE("{}({}): {}", Error::name(), e.errcode(), e.what());
    CLS_TRACE("req data: {}", getReqData());
    nng_http_res_set_header(m_nng_res, "Content-Type", "application/json; charset=UTF-8");
    nng_http_res_set_status(m_nng_res, e.status());
    nng_http_res_set_reason(m_nng_res, e.msg().c_str());
    nng_http_res_copy_data(m_nng_res, e.msg().c_str(), e.msg().size());
    nng_aio_set_output(m_http_aio, 0, m_nng_res);
    nng_aio_finish(m_http_aio, 0);
}

template <>
inline void HttpHandle::processHttpError<HttpError>(const HttpError &e) {
    CLS_TRACE("HttpError: {}", e.what());
    CLS_TRACE("req data: {}", getReqData());
    nng_http_res_set_header(m_nng_res, "Content-Type", "application/json; charset=UTF-8");
    nng_http_res_set_status(m_nng_res, e.status());
    nng_http_res_set_reason(m_nng_res, e.msg().c_str());
    nng_http_res_copy_data(m_nng_res, e.msg().c_str(), e.msg().size());
    nng_aio_set_output(m_http_aio, 0, m_nng_res);
    nng_aio_finish(m_http_aio, 0);
}

}  // namespace hku
