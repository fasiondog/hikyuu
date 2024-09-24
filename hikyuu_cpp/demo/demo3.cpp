/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * 这是利用 hikyuu 本身来实现一个数据缓存服务
 * 其接收 HikyuuTDX 行情采集发来的行情数据，并提供服务接口供其他程序来获取最新数据
 *
 * 用途：
 * 在程序化交易里，经常在实际下单时，希望获取最新数据，
 * 或者是日频交易时，不开启 hikyuu 本身的行情自动接收，而是在收盘前定时执行时，手工获取下最新数据
 *
 * hikyuu 中提供了对应的函数 get_data_from_buffer_server(python), getDataFromBufferServer(C++)
 * 用来从该服务获取最新数据（补齐当天数据）, 如：
 * get_data_from_buffer_server("tcp://192.168.1.1:9201", Query.DAY)
 *
 *************************************************************/

#include <hikyuu/hikyuu.h>
#include <thread>
#include <chrono>
#include <csignal>
#include <hikyuu/global/GlobalSpotAgent.h>
#include <hikyuu/utilities/node/NodeServer.h>
#include <hikyuu/utilities/os.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace hku;

NodeServer server;

void signal_handle(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        HKU_INFO("Shutdown now ...");
        server.stop();
        releaseGlobalSpotAgent();
        StockManager::quit();
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handle);
    std::signal(SIGTERM, signal_handle);

    initLogger(false, "./demo.log");

#if defined(_WIN32)
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        // 获取基础配置参数
        Parameter baseParam, blockParam, kdataParam, preloadParam, hkuParam;
        getConfigFromIni(fmt::format("{}/.hikyuu/hikyuu.ini", getUserDir()), baseParam, blockParam,
                         kdataParam, preloadParam, hkuParam);

        // 调整所有类型K线为预加载且预加载数量为1天的量
        Parameter new_preloadParam;
        auto ktypes = KQuery::getAllKType();
        for (auto& ktype : ktypes) {
            auto minutes = KQuery::getKTypeInMin(ktype);
            to_lower(ktype);
            new_preloadParam.set<bool>(ktype, true);
            if (minutes >= 240) {
                new_preloadParam.set<int>(fmt::format("{}_max", ktype), 1);
            } else {
                new_preloadParam.set<int>(fmt::format("{}_max", ktype), 240 / minutes);
                HKU_INFO("{}: {}", fmt::format("{}_max", ktype), 240 / minutes);
            }
        }
        // 不加载历史财务信息，不加载权息数据
        hkuParam.set<bool>("load_history_finance", false);
        hkuParam.set<bool>("load_stock_weight", false);
        StockManager::instance().init(baseParam, blockParam, kdataParam, new_preloadParam,
                                      hkuParam);

        // 启动行情接收
        startSpotAgent(true, 2);

        server.setAddr("tcp://0.0.0.0:9201");

        server.regHandle("market", [](json&& req) {
            HKU_INFO("--> req from {}:{}", req["remote_host"].get<string>(),
                     req["remote_port"].get<int>());
            HKU_ASSERT(req.contains("ktype"));
            HKU_ASSERT(req.contains("codes"));

            string ktype = req["ktype"].get<string>();
            auto& sm = StockManager::instance();
            const auto& param = sm.getPreloadParameter();
            string low_ktype = ktype;
            to_lower(low_ktype);
            HKU_CHECK(param.tryGet<bool>(low_ktype, false), "The ktype: {} is not be preloaded!",
                      ktype);

            const auto& jcodes = req["codes"];
            const auto& jdates = req["dates"];
            HKU_CHECK(jcodes.size() == jdates.size(), "The leght of codes and dates is not equal!");

            json jstklist;
            for (size_t i = 0, len = jcodes.size(); i < len; i++) {
                try {
                    string market_code = jcodes[i].get<string>();
                    Datetime start_date = Datetime(jdates[i].get<string>());
                    Stock stk = getStock(market_code);
                    if (stk.isNull()) {
                        HKU_DEBUG("Not found stock: {}", market_code);
                        continue;
                    }

                    KData kdata = stk.getKData(KQueryByDate(start_date, Null<Datetime>(), ktype));
                    if (kdata.empty()) {
                        continue;
                    }

                    json jklist;
                    for (const auto& k : kdata) {
                        json jr;
                        jr.emplace_back(k.datetime.str());
                        jr.emplace_back(k.openPrice);
                        jr.emplace_back(k.highPrice);
                        jr.emplace_back(k.lowPrice);
                        jr.emplace_back(k.closePrice);
                        jr.emplace_back(k.transAmount);
                        jr.emplace_back(k.transCount);
                        jklist.emplace_back(std::move(jr));
                    }

                    json jstk;
                    jstk["code"] = std::move(market_code);
                    jstk["data"] = std::move(jklist);
                    jstklist.emplace_back(std::move(jstk));

                } catch (const std::exception& e) {
                    HKU_ERROR("{}! The error occurred in record: {}", e.what(), i);
                }
            }

            json res;
            res["data"] = std::move(jstklist);
            // HKU_INFO("<-- res: {}", to_string(res));
            return res;
        });

        server.start();

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }

    server.stop();
    releaseGlobalSpotAgent();
    StockManager::quit();

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
