/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <string_view>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>

#include "../common/log.h"

namespace hku {

/**
 * 处理异常
 */
class HttpHandleRunException : public std::exception {
public:
    explicit HttpHandleRunException(int errcode) : std::exception(), m_errcode(errcode) {}

    int errcode() const {
        return m_errcode;
    }

private:
    int m_errcode = 0;
};

#define HANDLE_ERROR(errcode) throw HttpHandleRunException(errcode)

#define HANDLE_CHECK(expr, errcode)                \
    do {                                           \
        if (!expr) {                               \
            throw HttpHandleRunException(errcode); \
        }                                          \
    } while (0)

// 仅内部使用
#define NNG_CHECK(rv)                                       \
    {                                                       \
        if (rv != 0) {                                      \
            HKU_THROW("[HTTP_ERROR] {}", nng_strerror(rv)); \
        }                                                   \
    }

// 仅内部使用
#define NNG_CHECK_M(rv, msg)                                             \
    {                                                                    \
        if (rv != 0) {                                                   \
            HKU_THROW("[HTTP_ERROR] {} err: {}", msg, nng_strerror(rv)); \
        }                                                                \
    }

class HttpHandle {
    CLASS_LOGGER(HttpHandle)

public:
    HttpHandle() = delete;
    HttpHandle(nng_aio *aio);

    /** 前处理 */
    virtual void before_run() {}

    /** 响应处理 */
    virtual void run() = 0;

    // virtual void after_run() {}

    /** 统一处理响应错误 */
    virtual void error(int errcode) {
        unknown_error(fmt::format("error: {}", errcode));
    }

    /**
     * 获取请求头部信息
     * @param name 头部信息名称
     * @return 如果获取不到将返回 NULL
     */
    const char *getReqHeader(const char *name) {
        return nng_http_req_get_header(m_nng_req, name);
    }

    std::string getReqHeader(const std::string &name) {
        const char *head = nng_http_req_get_header(m_nng_req, name.c_str());
        return head ? std::string(head) : std::string();
    }

    void getReqData(void **data, size_t *len) {
        nng_http_req_get_data(m_nng_req, data, len);
    }

    std::string getReqData();

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

    void setResData(const std::string_view &content) {
        NNG_CHECK(nng_http_res_copy_data(m_nng_res, content.data(), content.size()));
    }

    void operator()();

private:
    // error未捕获的信息，统一返回500页面
    void unknown_error(const std::string &errmsg);

protected:
    nng_aio *m_http_aio{nullptr};
    nng_http_res *m_nng_res{nullptr};
    nng_http_req *m_nng_req{nullptr};
    nng_http_conn *m_nng_conn{nullptr};
};

}  // namespace hku
