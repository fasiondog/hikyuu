/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-26
 *     Author: fasiondog
 */

#include <hikyuu/hikyuu.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "http/HttpServer.h"

using namespace hku;

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    HttpServer server("http://*", 8080);
    server.start();

    SetConsoleOutputCP(old_cp);
    return 0;
}
