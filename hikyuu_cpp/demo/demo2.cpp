/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * 该示例使用 C++ 方式执行 Strategy 运行时
 * 也就是最常见的量化框架：定时调度 + 数据推送通知回调
 * 更多信息可参考 python 版 strategy 子目录中的示例
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
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 以多线程的方式执行多个策略
    // 注意：同一进程内的所有 strategy 共享的是同一个上下文！！！
    StrategyContext context({"sh000001", "sz000001"}, {KQuery::DAY});

    Strategy stg(context, "test");

    // stock 数据变化接收，通常用于调测，直接一般不需要
    stg.onChange(changed);

    // 每日开盘期间，按间隔时间循环执行
    stg.runDaily(my_process1, Minutes(1));

    // 每日定点执行
    stg.runDailyAt(my_process2, Datetime::now() - Datetime::today() + Seconds(20));

    auto t = std::thread([context]() {
        Strategy stg2(context, "stratege2");
        stg2.onChange(changed2);
        stg2.start();
    });

    // 启动策略
    stg.start();

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
