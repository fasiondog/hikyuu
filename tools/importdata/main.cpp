/*
 * main.cpp
 *
 *  Created on: 2010-10-26
 *      Author: fasiondog
 */

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <iniparser/IniParser.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

#include "importdata.h"

using namespace hku;

void import_data_by_configure(const SqlitePtr& db,
                              const hku::IniParser& ini_parser,
                              const std::string& section) {

    if (!ini_parser.hasSection(section))
        return;

    if (!ini_parser.hasOption(section, "dest"))
        return;

    void (*import_func)(const SqlitePtr&, const H5FilePtr&, const std::string&, const fs::path&);
    std::string market;
    if (section == "sh_day") {
        import_func = import_day_data;
        market = "SH";
    } else if (section == "sz_day") {
        import_func = import_day_data;
        market = "SZ";
    } else if (section == "sh_min5") {
        import_func = import_min5_data;
        market = "SH";
    } else if (section == "sz_min5") {
        import_func = import_min5_data;
        market = "SZ";
    } else if (section == "sh_min1") {
        import_func = import_min1_data;
        market = "SH";
    } else if (section == "sz_min1") {
        import_func = import_min1_data;
        market = "SZ";
    } else {
        std::cout << "Can't process section: " << section << std::endl;
        return;
    }

    std::string dest_file_name = ini_parser.get(section, "dest");
    H5FilePtr h5file = h5_open_file(dest_file_name);

    IniParser::StringListPtr option_list = ini_parser.getOptionList(section);
    IniParser::StringList src_list;
    IniParser::StringList::const_iterator iter;
    for (iter = option_list->begin(); iter != option_list->end(); ++iter) {
        if (*iter != "dest") {
            src_list.push_back(*iter);
        }
    }

    for (iter = src_list.begin(); iter != src_list.end(); ++iter) {
        std::string src_path = ini_parser.get(section, *iter);
        std::cout << "导入数据[" << market << "]: " << src_path << std::endl;
        MY_TIME_VALUE start_time, end_time;
        gettimeofday(&start_time, NULL);
        import_func(db, h5file, market, src_path);
        gettimeofday(&end_time, NULL);
        std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
    }
}

int main() {
    MY_TIME_VALUE total_start_time, total_end_time;
    MY_TIME_VALUE start_time, end_time;
    gettimeofday(&total_start_time, NULL);

#if defined(WIN32) || defined(_WIN32)
    char *home_path;
    size_t home_path_len;
    getenv_s(&home_path_len, NULL, 0, "HOMEPATH");
    home_path = (char*) malloc(home_path_len * sizeof(char) + 1);
    getenv_s( &home_path_len, home_path, home_path_len, "HOMEPATH" );
    std::string ini_file_name = "c:\\" + std::string(home_path) + "\\_cstock\\importdata.ini";
#else
    char *home_path = getenv("HOME");
    std::string ini_file_name = std::string(home_path) + "/.cstock/importdata.ini";
#endif

    hku::IniParser ini_parser;
    try {
        ini_parser.read(ini_file_name);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    if (!ini_parser.hasOption("database", "dest")) {
        std::cerr << "Cant't find optino [database] dest\n";
        return 0;
    }

    std::string db_name = ini_parser.get("database", "dest");

    SqlitePtr db;
    if (access(db_name.c_str(), 0)) {
        db = open_db(db_name);
        if (db) {
            if (!ini_parser.hasOption("database", "sql")) {
                std::cerr << "Can't find option [database]: " << db_name << std::endl;
                return 0;
            }
            create_database(db, ini_parser.get("database", "sql"));
        }
    } else {
        db = open_db(db_name);
    }

    H5::Exception::dontPrint();

    IniParser::StringListPtr option_list;
    IniParser::StringList src_list;
    IniParser::StringList::const_iterator iter;

    //导入上证股票信息
    if (ini_parser.hasSection("sh_day")) {
        src_list.clear();
        option_list = ini_parser.getOptionList("sh_day");
        for (iter = option_list->begin(); iter != option_list->end(); ++iter) {
            src_list.push_back(*iter);
        }

        for (iter = src_list.begin(); iter != src_list.end(); ++iter) {
            if (*iter != "dest") {
                std::string src_path = ini_parser.get("sh_day", *iter);
                std::cout << "导入上证股票信息: " << src_path << std::endl;;
                gettimeofday(&start_time, NULL);
                import_stock_info(db, "SH", src_path);
                gettimeofday(&end_time, NULL);
                std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
            }
        }
    }

    //导入深证股票信息
    if (ini_parser.hasSection("sz_day")) {
        src_list.clear();
        option_list = ini_parser.getOptionList("sz_day");
        for (iter = option_list->begin(); iter != option_list->end(); ++iter) {
            src_list.push_back(*iter);
        }

        for (iter = src_list.begin(); iter != src_list.end(); ++iter) {
            if (*iter != "dest") {
                std::string src_path = ini_parser.get("sz_day", *iter);
                std::cout << "导入深证股票信息: " << src_path << std::endl;;
                gettimeofday(&start_time, NULL);
                import_stock_info(db, "SZ", src_path);
                gettimeofday(&end_time, NULL);
                std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
            }
        }
    }

    //导入股票名称
    if (ini_parser.hasOption("database", "name")) {
        std::string stock_name_file = ini_parser.get("database", "name");
        std::cout << "导入股票名称: " << stock_name_file << std::endl;;
        gettimeofday(&start_time, NULL);
        import_stock_name(db, stock_name_file);
        gettimeofday(&end_time, NULL);
        std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
    }

    //导入上证权息信息
    if (ini_parser.hasOption("weight", "sh")) {
        std::string src_path = ini_parser.get("weight", "sh");
        std::cout << "导入上证权息信息: " << src_path << std::endl;
        gettimeofday(&start_time, NULL);
        import_stock_weight(db, "SH", src_path);
        gettimeofday(&end_time, NULL);
        std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
    }

    //导入深证权息信息
    if (ini_parser.hasOption("weight", "sz")) {
        std::string src_path = ini_parser.get("weight", "sz");
        std::cout << "导入深证权息信息: " << src_path << std::endl;
        gettimeofday(&start_time, NULL);
        import_stock_weight(db, "SZ", src_path);
        gettimeofday(&end_time, NULL);
        std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
    }

    import_data_by_configure(db, ini_parser, "sh_day");
    import_data_by_configure(db, ini_parser, "sz_day");
    import_data_by_configure(db, ini_parser, "sh_min5");
    import_data_by_configure(db, ini_parser, "sz_min5");
    import_data_by_configure(db, ini_parser, "sh_min1");
    import_data_by_configure(db, ini_parser, "sz_min1");

    std::cout << "更新股票日期信息...\n";
    gettimeofday(&start_time, NULL);
    if (ini_parser.hasOption("sh_day", "dest")) {
        std::string dest_file_name = ini_parser.get("sh_day", "dest");
        H5FilePtr sh_day_h5file = h5_open_file(dest_file_name);
        update_all_stock_date(db, sh_day_h5file, "SH");
    }
    if (ini_parser.hasOption("sz_day", "dest")) {
        std::string dest_file_name = ini_parser.get("sz_day", "dest");
        H5FilePtr sz_day_h5file = h5_open_file(dest_file_name);
        update_all_stock_date(db, sz_day_h5file, "SZ");
    }
    gettimeofday(&end_time, NULL);
    std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;

    gettimeofday(&total_end_time, NULL);
    double total_time = mydifftime(total_end_time, total_start_time);
    std::cout << "总耗时：" << total_time/60.0 << "m " << total_time << "s" << std::endl << std::endl;

    return 0;
}
