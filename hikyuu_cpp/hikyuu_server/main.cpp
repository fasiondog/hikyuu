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
#include "rest_api/login.h"

using namespace hku;

int main(int argc, char* argv[]) {
    init_server_logger();

    LOG_INFO("start server ... You can press Ctrl-C stop");

    HttpServer server("http://*", 520);

    server.GET<HelloHandle>("/hello");
    server.GET<LoginHandle>("/login");

    server.start();
    return 0;
}
