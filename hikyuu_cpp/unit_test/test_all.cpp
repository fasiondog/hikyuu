/*
 * test_all.cpp
 *
 *  Created on: 2010-5-23
 *      Author: fasiondog
 */

/**
 * @defgroup test_all test_all
 * 该文件的主要目的是在集成IDE环境中，可以将所有测试用例集合在一起作为一个测试程序进行编译
 * 编译时，需要在编译选项中声明宏TEST_ALL_IN_ONE （一般为-DTEST_ALL_IN_ONE）
 * 原因：boost::unit_test框架的习惯用法是将一个CPP文件最为一个可运行的测试模块独立生成
 *      一个可执行文件，并在bjam编译时自动运行。但是在IDE开发环境中，要定义多个编译输出
 *      需要对应建立不同的项目，即使这个项目非常小。这样在IDE中，使用时很不方便，也不利于
 *      直接使用IDE环境进行编译。
 *
 * @note \arg 使用IDE环境中主要是为了快速检查编译错误，所以这个文件将来很可能不维护，而仅仅
 *            用来快速解决编译问题。
 *       \arg 要在编译选项中声明TEST_ALL_IN_ONE宏
 *
 * @see test_all.cpp
 */

#define BOOST_TEST_MODULE test_all
#include <boost/test/unit_test.hpp>

// Linux下在Eclipse中编译必须使用下面的头文件（使用bjam不需要），否则提示无法找到“main“函数，原因未知
// 但bjam编译时会有告警，虽然可以编译通过。
// Windows下，如果使用下面的头文件将无法编译。
#ifdef ECLIPSE_GCC
#include <boost/test/included/unit_test.hpp>
#endif

using namespace boost::unit_test;
