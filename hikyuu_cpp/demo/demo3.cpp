// demo.cpp : 定义控制台应用程序的入口点。
//

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
        // 配置文件的位置自行修改
        hikyuu_init(fmt::format("{}/.hikyuu/hikyuu.ini", getUserDir()));

        // 启动行情接收（只是计算回测可以不需要）
        startSpotAgent(true);

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

            json data;
            const auto& jcodes = req["codes"];
            // HKU_INFO("codes size: {}", jcodes.size());
            for (auto iter = jcodes.cbegin(); iter != jcodes.cend(); ++iter) {
                string market_code = to_string(*iter);
                market_code = market_code.substr(1, market_code.size() - 2);
                Stock stk = getStock(market_code);
                if (stk.isNull()) {
                    HKU_WARN("Not found stock: {}", market_code);
                    continue;
                }

                KRecordList krecords =
                  stk.getKRecordList(KQueryByIndex(-1, Null<int64_t>(), ktype));
                if (!krecords.empty()) {
                    const auto& k = krecords.back();
                    json jr;
                    jr.emplace_back(market_code);
                    jr.emplace_back(k.datetime.str());
                    jr.emplace_back(k.openPrice);
                    jr.emplace_back(k.highPrice);
                    jr.emplace_back(k.lowPrice);
                    jr.emplace_back(k.closePrice);
                    jr.emplace_back(k.transAmount);
                    jr.emplace_back(k.transCount);
                    data.emplace_back(std::move(jr));
                }
            }

            json res;
            res["data"] = data;
            // HKU_INFO("<-- res: {}", to_string(res));
            return res;
        });

        server.start();
        // server.loop();
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }

    server.stop();

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
