/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <unordered_set>
#include <hikyuu/utilities/thread/ThreadPool.h>
#include <hikyuu/utilities/thread/FuncWrapper.h>
#include "HttpHandle.h"

namespace hku {

class HttpServer {
public:
    HttpServer(const char *host, uint16_t port);
    virtual ~HttpServer();

    void start();

    template <typename Handle>
    void GET(const char *path) {
        regHandle("GET", path, [](nng_aio *aio) { submit(Handle(aio)); });
    }

    template <typename Handle>
    void POST(const char *path) {
        regHandle("POST", path, [](nng_aio *aio) { submit(Handle(aio)); });
    }

    template <typename Handle>
    void regHandle(const char *method, const char *path) {
        regHandle(method, path, [](nng_aio *aio) { submit(Handle(aio)); });
    }

private:
    void regHandle(const char *method, const char *path, void (*rest_handle)(nng_aio *));

private:
    std::string m_root_url;
    std::string m_host;
    uint16_t m_port;

private:
    static void http_exit();
    static void signal_handler(int signal);

    /** 先线程池提交任务后返回的对应 future 的类型 */
    template <typename ResultType>
    using task_handle = std::future<ResultType>;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    /** 向线程池提交任务 */
    template <typename FunctionType>
    static task_handle<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        return ms_tg.submit(f);
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

private:
    static nng_http_server *ms_server;
    static ThreadPool ms_tg;
};

}  // namespace hku