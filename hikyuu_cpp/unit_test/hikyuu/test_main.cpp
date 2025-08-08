/*
 * Copyright(C) hikyuu.org
 *
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include <hikyuu/config.h>
#if !HKU_ENABLE_HDF5_KDATA
#error test program only for hdf5 engine! You must config --hdf5=y
#endif

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <hikyuu/hikyuu.h>
#include <hikyuu/utilities/os.h>
#include <hikyuu/debug.h>
using namespace hku;

#ifdef HKU_USE_REAL_DATA_TEST
void init_hikyuu_test() {
    set_log_level(LOG_LEVEL::LOG_TRACE);
    std::string config_file(fmt::format("{}/.hikyuu/hikyuu.ini", getUserDir()));
    fmt::print("configure file: {}\n", config_file);
    hikyuu_init(config_file);
    StockManager& sm = StockManager::instance();
    createDir(sm.tmpdir());
}
#else
void init_hikyuu_test() {
    set_log_level(LOG_LEVEL::LOG_TRACE);

    auto current = fmt::format("{}/test_data", getCurrentDir());
    fmt::print("current path: {}\n", current);

#if HKU_OS_WINDOWS
    std::string config_file(fmt::format("{}\\hikyuu_win.ini", current));
#else
    std::string config_file(fmt::format("{}/hikyuu_linux.ini", current));
#endif

    auto plugin_path = fmt::format("{}/plugin", getCurrentDir());
    StockManager::instance().setPluginPath(plugin_path);
    fmt::print("current plugin path: {}\n", plugin_path);

    fmt::print("configure file: {}\n", config_file);
    hikyuu_init(config_file);

    std::string tmp_dir(fmt::format("{}/tmp", current));
    createDir(tmp_dir);
}
#endif

int main(int argc, char** argv) {
#if defined(_WIN32)
    // Windows 下设置控制台程序输出代码页为 UTF8
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    doctest::Context context;

    // !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

    // defaults
    // context.addFilter("test-case-exclude", "*math*");  // exclude test cases with "math" in their
    // name
    context.setOption("abort-after", 5);    // stop test execution after 5 failed assertions
    context.setOption("order-by", "name");  // sort the test cases by their name

    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("no-breaks", true);  // don't break in the debugger when assertions fail

    init_hikyuu_test();

    int res = 0;
    {
        SPEND_TIME_MSG(total_test_run, "Total test time");
        res = context.run();  // run
        std::cout << std::endl;
    }

    if (context.shouldExit())  // important - query flags (and --exit) rely on the user doing this
        return res;            // propagate the result of the tests

    int client_stuff_return_code = 0;
    // your program - if the testing framework is integrated in your production code

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif

    return res + client_stuff_return_code;  // the result from doctest is propagated here as well
}