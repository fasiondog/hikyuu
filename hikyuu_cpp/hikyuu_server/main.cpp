/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-26
 *     Author: fasiondog
 */

#include <locale>
#include <csignal>
#include <hikyuu/utilities/os.h>
#include "http/HttpServer.h"
#include "db/db.h"
#include "service/user/UserService.h"
#include "service/assist/AssistService.h"
#include "service/trade/TradeService.h"

using namespace hku;

#define HKU_SERVICE_API(name) "/hku/" #name "/v1"

void signal_handle(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        APP_INFO("Shutdown now ...");
        HttpServer::stop();
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    init_server_logger();

    // 初始化多语言支持
    MOHelper::init();

    std::signal(SIGINT, signal_handle);
    std::signal(SIGTERM, signal_handle);

    HttpServer server("http://*", 9001);
    HttpHandle::enableTrace(true, false);

    try {
        // 设置 404 返回信息
        server.set_error_msg(
          NNG_HTTP_STATUS_NOT_FOUND,
          fmt::format(R"({{"result": false,"errcode":{}, "errmsg":"Not Found"}})",
                      int(NNG_HTTP_STATUS_NOT_FOUND)));

        DB::init(fmt::format("{}/.hikyuu/trade.ini", getUserHome()));

        UserService usr_service(HKU_SERVICE_API(user));
        usr_service.bind(&server);

        AssistService assist(HKU_SERVICE_API(assist));
        assist.bind(&server);

        TradeService trade(HKU_SERVICE_API(trade));
        trade.bind(&server);

        APP_INFO("start server ... You can press Ctrl-C stop");
        server.start();

    } catch (std::exception& e) {
        APP_FATAL(e.what());
        server.stop();
    } catch (...) {
        APP_FATAL("Unknow error!");
        server.stop();
    }

    return 0;
}
