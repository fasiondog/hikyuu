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

    /** 获取请求的 url */
    std::string getReqUrl() const;

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

    /** 根据 Content-Encoding 进行解码，返回解码后的请求数据 */
    std::string getReqData();

    /** 返回请求的 json 数据，如无法解析为json，将抛出异常*/
    json getReqJson();

    /** 判断请求的 ulr 中是否包含 query 参数 */
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

    /** 获取当前的相应数据 */
    std::string getResData() const;

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
    void processException(int http_status, int errcode, std::string_view err_msg);

protected:
    nng_aio *m_http_aio{nullptr};
    nng_http_res *m_nng_res{nullptr};
    nng_http_req *m_nng_req{nullptr};
    nng_http_conn *m_nng_conn{nullptr};
    std::vector<std::function<void(HttpHandle *)>> m_filters;

public:
    static void enableTrace(bool enable, bool only_traceid = false) {
        ms_enable_trace = enable;
        ms_enable_only_traceid = only_traceid;
    }

protected:
    void trace();
    std::string getTraceInfo();

private:
    // 是否跟踪请求打印
    inline static std::atomic_bool ms_enable_trace = false;
    inline static std::atomic_bool ms_enable_only_traceid = false;
};

#define HTTP_HANDLE_IMP(cls) \
public:                      \
    cls(nng_aio *aio) : HttpHandle(aio) {}

}  // namespace hku
