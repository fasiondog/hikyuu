/*
 * config.cpp
 *
 *  Created on: 2011-2-20
 *      Author: fasiondog
 */

#include "config.h"
#include <iostream>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

GConfig::GConfig() {
    start_time = std::chrono::system_clock::now();

    path current = current_path();
    if ( current.stem() == path("test") )  {
        current /= path("data");
    } else {
        current /= "test_data";
    }

    std::cout << "current path  : " << current << std::endl;
#if defined(BOOST_WINDOWS)
    std::cout << "configure file: " << current.string() << "\\hikyuu_win.ini" << std::endl;
    hikyuu_init(current.string() + "\\hikyuu_win.ini");
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

GConfig::~GConfig() {
    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start_time;
    std::cout << "All test spend time: " << sec.count() << "s" << std::endl;
}

BOOST_GLOBAL_FIXTURE(GConfig);
