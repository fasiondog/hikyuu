/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-26
 *     Author: fasiondog
 */

#include <hikyuu/hikyuu.h>
#include "http/HttpServer.h"
#include "service/login/LoginService.h"
#include "service/assist/AssistService.h"

using namespace hku;

#define HKU_SERVICE_API(name) "/hku/" #name "/v1"

int main(int argc, char* argv[]) {
    init_server_logger();

    LOG_INFO("start server ... You can press Ctrl-C stop");

    HttpServer server("http://*", 9001);

    LoginService login(HKU_SERVICE_API(login));
    login.bind(&server);

    AssistService assist(HKU_SERVICE_API(assist));
    assist.bind(&server);

    server.start();
    return 0;
}
