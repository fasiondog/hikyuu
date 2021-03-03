/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-26
 *     Author: fasiondog
 */

#include <hikyuu/hikyuu.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "http/HttpServer.h"
#include "rest_api/hello.h"

//#include <hv/HttpServer.h>

using namespace hku;

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    HKU_INFO("start server ... You can press Ctrl-C stop");
    HKU_INFO("main: {}", std::this_thread::get_id());

    HttpServer server("http://*", 8080);
    server.GET<HelloHandle>("/hello");
    server.start();

        /*HttpService router;
    router.GET("/hello", [](HttpRequest* req, HttpResponse* resp) {
        HKU_INFO("work: {}", std::this_thread::get_id());
        return resp->String("hello");
    });

    http_server_t server;
    server.port = 8080;
    server.worker_threads = std::thread::hardware_concurrency();
    // uncomment to test multi-processes
    // server.worker_processes = 4;
    // uncomment to test multi-threads
    // server.worker_threads = 4;
    server.service = &router;

    http_server_run(&server);*/

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
