/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * This implements a data cache service with hikyuu itself
 * It receives the market data sent by the HikyuuTDX collector and offers an interface for the other
 * programs to get the latest data
 *
 * Purpose:
 * In the programmatic trading the latest data is often wanted when placing a real order,
 * or in daily trading the automatic receiving of hikyuu is off and the latest data is fetched
 * manually during a scheduled run before the close
 *
 * hikyuu provides get_data_from_buffer_server (python) and getDataFromBufferServer (C++)
 * to get the latest data from this service (filling the data of the day), e.g.:
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
    // Set the console output code page to UTF8 on Windows
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    // The plugin path setting:
    // Method 1: before the initialization, set the plugin path to "." or "" and it is taken
    // automatically from the plugindir of the hikyuu.ini config:
    // StockManager::instance().setPluginPath("."); Method 2: before the initialization, set the
    // plugin path yourself (if needed) otherwise it defaults to the .hikyuu/plugin directory under
    // the user home, where the plugins can be copied
    // StockManager::instance().setPluginPath("./plugin");

    try {
        // Get the basic configuration parameters
        Parameter baseParam, blockParam, kdataParam, preloadParam, hkuParam;
        getConfigFromIni(fmt::format("{}/.hikyuu/hikyuu.ini", getUserDir()), baseParam, blockParam,
                         kdataParam, preloadParam, hkuParam);

        // Preload every K-line type with a preload amount of one day
        Parameter new_preloadParam;
        auto ktypes = KQuery::getBaseKTypeList();
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
        // Do not load the historical financial information nor the ex-rights/ex-dividend data
        hkuParam.set<bool>("load_history_finance", false);
        hkuParam.set<bool>("load_stock_weight", false);
        StockManager::instance().init(baseParam, blockParam, kdataParam, new_preloadParam,
                                      hkuParam);

        // Start the market data receiving
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
