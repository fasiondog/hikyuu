// demo.cpp : 定义控制台应用程序的入口点。
//

#include <hikyuu/hikyuu.h>
#include <thread>
#include <chrono>
#include <hikyuu/global/GlobalSpotAgent.h>

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
    hikyuu_init("C:\\Users\\admin\\.hikyuu\\hikyuu.ini");
    // hikyuu_init("/home/fsd/.hikyuu/hikyuu.ini");

    StockManager& sm = StockManager::instance();

    Stock stk = sm.getStock("sh000001");
    std::cout << stk << std::endl;

    auto k = stk.getKData(KQuery(-10));
    std::cout << k << std::endl;

    for (size_t i = 0; i < k.size(); i++) {
        std::cout << k[i] << std::endl;
    }

    startSpotAgent(true);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
