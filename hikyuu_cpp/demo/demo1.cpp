/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * 该示例，为使用 C++ 方式使用 hikyuu
 * 1. 初始化 hikyuu
 * 2. 打印 K 线数据
 * 更多使用可以参考 python，基本一致，仅函数命名风格不一样
 *
 *************************************************************/

#include <hikyuu/hikyuu.h>
#include <thread>
#include <chrono>
#include <hikyuu/global/GlobalSpotAgent.h>
#include <hikyuu/utilities/os.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace hku;

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 配置文件的位置自行修改
    hikyuu_init(fmt::format("{}/.hikyuu/hikyuu.ini", getUserDir()));

    StockManager& sm = StockManager::instance();

    Stock stk = sm.getStock("sh000001");
    std::cout << stk << std::endl;

    auto k = stk.getKData(KQuery(-10));
    std::cout << k << std::endl;

    for (size_t i = 0; i < k.size(); i++) {
        std::cout << k[i] << std::endl;
    }

    // 启动行情接收（只是计算回测可以不需要）
    // startSpotAgent(true);
    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
