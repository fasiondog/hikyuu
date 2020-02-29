/*
 * Copyright(C) hikyuu.org
 *
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#if __GNUC__ <= 8 || __clang_major__ <= 6
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
#else
#include <filesystem>
using namespace std::filesystem;
#endif

#include <hikyuu/hikyuu.h>
#include <hikyuu/debug.h>
using namespace hku;

void init_hikyuu_test() {
    path current = current_path();
    if (current.stem() == path("test")) {
        current /= path("data");
    } else {
        current /= "test_data";
    }

    std::cout << "current path  : " << current << std::endl;
#if defined(_MSC_VER)
    std::cout << "configure file: " << current.string() << "\\hikyuu_win.ini" << std::endl;
    hku::hikyuu_init(current.string() + "\\hikyuu_win.ini");
#else
    std::cout << "configure file: " << current.string() << "/hikyuu_linux.ini" << std::endl;
    hikyuu_init(current.string() + "/hikyuu_linux.ini");
#endif

    path tmp_dir = current;
    tmp_dir /= "tmp";
    if (!exists(tmp_dir)) {
        create_directory(tmp_dir);
    }
}

int main(int argc, char** argv) {
    doctest::Context context;

    // !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

    // defaults
    // context.addFilter("test-case-exclude", "*math*");  // exclude test cases with "math" in their name
    context.setOption("abort-after", 5);    // stop test execution after 5 failed assertions
    context.setOption("order-by", "name");  // sort the test cases by their name

    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("no-breaks", true);  // don't break in the debugger when assertions fail

    init_hikyuu_test();

    int res = 0;
    {
        SPEND_TIME_MSG(total_test_run, utf8_to_gb("测试总耗时"));
        res = context.run();  // run
        std::cout << std::endl;
    }

    std::cout << std::endl;

    StockManager::quit();
    spdlog::drop_all();

    if (context.shouldExit())  // important - query flags (and --exit) rely on the user doing this
        return res;            // propagate the result of the tests

    int client_stuff_return_code = 0;
    // your program - if the testing framework is integrated in your production code

    return res + client_stuff_return_code;  // the result from doctest is propagated here as well
}