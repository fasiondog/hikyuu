/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * This example runs the Strategy runtime in C++
 * i.e. the most common quant framework: a scheduled dispatch + a data push callback
 * More information can be found in the python strategy examples subdirectory
 *
 *************************************************************/

#include <hikyuu/hikyuu.h>
#include <thread>
#include <chrono>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace hku;

static void changed(Strategy* stg, const Stock& stk, const SpotRecord& spot) {
    HKU_INFO("{} {} 当前收盘价: {}", stk.market_code(), stk.name(), spot.close);
}

static void changed2(Strategy* stg, const Stock& stk, const SpotRecord& spot) {
    if (stk.market_code() == "SZ000001") {
        HKU_INFO("strategy 2 process sz000001");
    }
}

static void my_process1(Strategy* stg) {
    HKU_INFO("{}", getStock("sh000001"));
}

static void my_process2(Strategy* stg) {
    HKU_INFO("run at time: {} {}", Datetime::now(), getStock("sh000001").name());
}

int main(int argc, char* argv[]) {
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

    // Run multiple strategies in a multi-threaded way
    // Note: all the strategies in the same process share the same context!!!
    StrategyContext context({"sh000001", "sz000001"}, {KQuery::DAY});

    // On macosx an explicit init is needed for multi-threaded strategies, avoiding a plugin load
    // failure in an async thread
    hikyuu_init(context, true);

    Strategy stg(context, "test");

    // The stock data change receiving, usually for debugging and not needed normally
    stg.onChange(changed);

    // Execute in a loop at an interval during the daily open
    stg.runDaily(my_process1, Minutes(1));

    // Execute at a fixed time every day
    stg.runDailyAt(my_process2, Datetime::now() - Datetime::today() + Seconds(20));

    auto t = std::thread([context]() {
        Strategy stg2(context, "stratege2");
        stg2.onChange(changed2);
        stg2.start();
    });

    // Start the strategy
    stg.start();

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
