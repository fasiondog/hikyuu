/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>

#include "../common/log.h"

namespace hku {

#if !defined(__clang__) && !defined(__GNUC__)
class HttpHandleException : public std::exception {
public:
    HttpHandleException() : std::exception("Unknown exception!") {}
    HttpHandleException(int http_status, const std::string &msg)
    : std::exception(msg.c_str()), m_http_status(http_status) {}
    HttpHandleException(int http_status, const char *msg)
    : std::exception(msg), m_http_status(http_status) {}

    int status() const {
        return m_http_status;
    }

private:
    int m_http_status{NNG_HTTP_STATUS_BAD_REQUEST};
};

#else
// llvm 中的 std::exception 不接受参数
class HttpHandleException : public std::exception {
public:
    HttpHandleException() : m_msg("Unknown exception!") {}
    HttpHandleException(int http_status, const char *msg)
    : m_msg(msg), m_http_status(http_status) {}
    HttpHandleException(int http_status, const std::string &msg)
    : m_msg(msg), m_http_status(http_status) {}
    virtual ~HttpHandleException() noexcept {}
    virtual const char *what() const noexcept {
        return m_msg.c_str();
    }

    int status() const {
        return m_http_status;
    }

protected:
    std::string m_msg;
    int m_http_status{NNG_HTTP_STATUS_BAD_REQUEST};
};
#endif /* #ifdef __clang__ */

#define HANDLE_THROW(http_status, ...) \
    { throw HttpHandleException(http_status, fmt::format(__VA_ARGS__)); }

#define HANDLE_CHECK(expr, http_status, ...)                                  \
    {                                                                         \
        if (!expr) {                                                          \
            throw HttpHandleException(http_status, fmt::format(__VA_ARGS__)); \
        }                                                                     \
    }

// 仅内部使用
#define NNG_CHECK(rv)                                      \
    {                                                      \
        if (rv != 0) {                                     \
            HKU_THROW("[NNG_ERROR] {}", nng_strerror(rv)); \
        }                                                  \
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

    void addFilter(std::function<void(HttpHandle *)> filter) {
        m_filters.push_back(filter);
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
    std::vector<std::function<void(HttpHandle *)>> m_filters;
};

}  // namespace hku
