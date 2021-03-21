/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-26
 *     Author: fasiondog
 */

#include <hikyuu/hikyuu.h>
#include "http/HttpServer.h"
#include "rest_api/hello.h"
#include "service/login/LoginService.h"

using namespace hku;

int main(int argc, char* argv[]) {
    init_server_logger();

    LOG_INFO("start server ... You can press Ctrl-C stop");

    HttpServer server("http://*", 520);

    server.GET<HelloHandle>("/hello");

    LoginService login;
    login.bind(&server);

    server.start();
    return 0;
}
