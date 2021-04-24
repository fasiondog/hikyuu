/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-26
 *     Author: fasiondog
 */

#include <hikyuu/utilities/os.h>
#include "http/HttpServer.h"
#include "db/db.h"
#include "service/user/UserService.h"
#include "service/assist/AssistService.h"
#include "service/trade/TradeService.h"

using namespace hku;

#define HKU_SERVICE_API(name) "/hku/" #name "/v1"

int main(int argc, char* argv[]) {
    init_server_logger();
    DB::init(fmt::format("{}/.hikyuu/trade.ini", getUserHome()));

    LOG_INFO("start server ... You can press Ctrl-C stop");

    HttpServer server("http://*", 9001);

    try {
        UserService login(HKU_SERVICE_API(user));
        login.bind(&server);

        AssistService assist(HKU_SERVICE_API(assist));
        assist.bind(&server);

        TradeService trade(HKU_SERVICE_API(trade));
        trade.bind(&server);

        server.start();

    } catch (std::exception& e) {
        LOG_FATAL(e.what());
        server.stop();
    } catch (...) {
        LOG_FATAL("Unknow error!");
        server.stop();
    }

    return 0;
}
