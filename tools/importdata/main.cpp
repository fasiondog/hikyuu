/*
 * main.cpp
 *
 *  Created on: 2010-10-26
 *      Author: fasiondog
 */

#include <iostream>
#include <boost/filesystem.hpp>
//#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <iniparser/IniParser.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

#include "importdata.h"

using namespace hku;


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
    std::string data_config_file = "c:\\" + std::string(home_path) + "\\.hikyuu\\data_dir.ini";
#else
    char *home_path = getenv("HOME");
    std::string data_config_file = std::string(home_path) + "/.hikyuu/data_dir.ini";
#endif

    hku::IniParser data_config;
    try {
        data_config.read(data_config_file);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    if (!data_config.hasOption("data_dir", "data_dir")) {
        std::cerr << "Can't find option data_dir" << std::endl;
        return 0;
    }

    std::string ini_file_name = data_config.get("data_dir", "data_dir") + "/importdata.ini";

    hku::IniParser ini_parser;
    try {
        ini_parser.read(ini_file_name);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////
    ///
    /// 打开或创建数据库
    ///
    ///////////////////////////////////////////////////////////////////////

    if (!ini_parser.hasOption("database", "db")) {
        std::cerr << "Cant't find option [database] db\n";
        return 0;
    }

    std::string db_name = ini_parser.get("database", "db");
    std::cout << "目标数据库: " << db_name << std::endl;

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

    ///////////////////////////////////////////////////////////////////////
    ///
    /// 获取数据源配置信息
    ///
    ///////////////////////////////////////////////////////////////////////

    struct DataSource {
        std::string type;
        std::string path;
    };

    bool only_dzh = true; //如果仅有大智慧数据源标志，后续使用，因为大智慧不支持分钟线
    std::list<DataSource> from_list;
    IniParser::StringListPtr option_list;
    IniParser::StringList::const_iterator iter;
    if (ini_parser.hasSection("from")) {
        option_list = ini_parser.getOptionList("from");
        for (iter = option_list->begin(); iter != option_list->end(); ++iter) {
            std::string from = ini_parser.get("from", *iter);
            size_t pos = from.find(",");
            if (pos == std::string::npos) {
                std::cout << "Ingored error format: " << from << std::endl;
            } else {
                DataSource data_from;
                data_from.type = from.substr(0, pos);
                data_from.path = from.substr(pos + 1);
                boost::trim(data_from.type);
                boost::to_upper(data_from.type);
                boost::trim(data_from.path);
                if (data_from.type == "DZH") {
                    std::cout << "大智慧数据源: " << data_from.path << std::endl;
                    from_list.push_back(data_from);
                } else if (data_from.type == "TDX") {
                    std::cout << "通达信数据源: " << data_from.path << std::endl;
                    from_list.push_back(data_from);
                    only_dzh = false; //仅有大智慧数据源标志置为false
                } else {
                    std::cout << "不支持的数据源: " << data_from.path << std::endl;
                }
            }
        }
    } else {
        std::cout << "未配置数据源！" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////
    ///
    /// 获取目标HDF5配置信息
    ///
    ///////////////////////////////////////////////////////////////////////

    bool import_all = false;
    std::map<std::string, std::string> dest_dict;
    if (ini_parser.hasSection("dest")) {

        import_all = ini_parser.getBool("dest", "all", "false");
        if (import_all)
            std::cout << "导入所有K线数据，不论数据库是否存在该股票信息！！！" << std::endl;

        if (ini_parser.hasOption("dest", "sh_day")) {
            dest_dict["sh_day"] = ini_parser.get("dest", "sh_day");
            std::cout << "sh_day  目标文件: " << dest_dict["sh_day"] << std::endl;
        }
        if (ini_parser.hasOption("dest", "sz_day")) {
            dest_dict["sz_day"] = ini_parser.get("dest", "sz_day");
            std::cout << "sz_day  目标文件: " << dest_dict["sz_day"] << std::endl;
        }
        if (ini_parser.hasOption("dest", "sz_day")) {
            dest_dict["sh_5min"] = ini_parser.get("dest", "sh_5min");
            std::cout << "sh_5min 目标文件: " << dest_dict["sh_5min"] << std::endl;
        }
        if (ini_parser.hasOption("dest", "sz_5min")) {
            dest_dict["sz_5min"] = ini_parser.get("dest", "sz_5min");
            std::cout << "sz_5min 目标文件: " << dest_dict["sz_5min"] << std::endl;
        }

        if (!only_dzh) {
            if (ini_parser.hasOption("dest", "sh_1min")) {
                dest_dict["sh_1min"] = ini_parser.get("dest", "sh_1min");
                std::cout << "sh_1min 目标文件: " << dest_dict["sh_1min"] << std::endl;
            }
            if (ini_parser.hasOption("dest", "sh_1min")) {
                dest_dict["sz_1min"] = ini_parser.get("dest", "sz_1min");
                std::cout << "sz_1min 目标文件: " << dest_dict["sz_1min"] << std::endl;
            }
        } else {
            std::cout << "仅有大智慧数据源，忽略分钟线!" << std::endl;
        }

    } else {
        std::cout << "未配置目标HDF5文件！" << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////
    ///
    /// 导入股票信息（代码、名称）
    ///
    ///////////////////////////////////////////////////////////////////////

    std::cout << std::endl;

    std::list<DataSource>::const_iterator from_iter = from_list.begin();
    for (; from_iter != from_list.end(); ++from_iter) {
        if (from_iter->type == "DZH") {
            std::cout << "从大智慧导入股票代码表" << std::endl;
            gettimeofday(&start_time, NULL);
            dzh_import_stock_name(db, from_iter->path);
            gettimeofday(&end_time, NULL);
            std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;

        } else if (from_iter->type == "TDX") {
            std::cout << "从通达信导入股票代码表" << std::endl;
            gettimeofday(&start_time, NULL);
            tdx_import_stock_name(db, from_iter->path);
            gettimeofday(&end_time, NULL);
            std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
        }
    }


    ///////////////////////////////////////////////////////////////////////
    ///
    /// 导入K线数据
    ///
    ///////////////////////////////////////////////////////////////////////

    H5::Exception::dontPrint();

    std::map<std::string, std::string>::const_iterator dest_iter;
    std::map<std::string, H5FilePtr> h5file_dict;
    for (dest_iter = dest_dict.begin(); dest_iter != dest_dict.end(); ++dest_iter) {
        h5file_dict[dest_iter->first] = h5_open_file(dest_iter->second);
    }

    void (*import_func)(const SqlitePtr&, const H5FilePtr&, const std::string&, const fs::path&);

    for (from_iter = from_list.begin(); from_iter != from_list.end(); ++from_iter) {
        for (dest_iter = dest_dict.begin(); dest_iter != dest_dict.end(); ++dest_iter) {
            MY_TIME_VALUE start_time, end_time;
            gettimeofday(&start_time, NULL);

            bool need_import = true;
            std::string path, market;
            //大智慧导入
            if (from_iter->type == "DZH") {
                if (dest_iter->first == "sh_day") {
                    market = "SH";
                    path = from_iter->path + "/DATA/SHase/Day";
                    import_func = import_all ? dzh_import_all_day_data : dzh_import_day_data;

                } else if (dest_iter->first == "sz_day") {
                    market = "SZ";
                    path = from_iter->path + "/DATA/SZnse/Day";
                    import_func = import_all ? dzh_import_all_day_data : dzh_import_day_data;

                } else if (dest_iter->first == "sh_5min") {
                    market = "SH";
                    path = from_iter->path + "/DATA/SHase/Min";
                    import_func = import_all ? dzh_import_all_min5_data : dzh_import_min5_data;

                } else if (dest_iter->first == "sz_5min") {
                    market = "SZ";
                    path = from_iter->path + "/DATA/SZnse/Min";
                    import_func = import_all ? dzh_import_all_min5_data: dzh_import_min5_data;
                } else {
                    need_import = false;
                }

            //通达信导入
            } else if (from_iter->type == "TDX") {
                if (dest_iter->first == "sh_day") {
                    market = "SH";
                    path = from_iter->path + "/vipdoc/sh/lday";
                    import_func = import_all ? tdx_import_all_day_data : tdx_import_day_data;

                } else if (dest_iter->first == "sz_day") {
                    market = "SZ";
                    path = from_iter->path + "/vipdoc/sz/lday";
                    import_func = import_all ? tdx_import_all_day_data : tdx_import_day_data;

                } else if (dest_iter->first == "sh_5min") {
                    market = "SH";
                    path = from_iter->path + "/vipdoc/sh/fzline";
                    import_func = import_all ? tdx_import_all_min_data : tdx_import_min_data;

                } else if (dest_iter->first == "sz_5min") {
                    market = "SZ";
                    path = from_iter->path + "/vipdoc/sz/fzline";
                    import_func = import_all ? tdx_import_all_min_data : tdx_import_min_data;

                } else if (dest_iter->first == "sh_1min") {
                    market = "SH";
                    path = from_iter->path + "/vipdoc/sh/minline";
                    import_func = import_all ? tdx_import_all_min_data : tdx_import_min_data;

                } else if (dest_iter->first == "sz_1min") {
                    market = "SZ";
                    path = from_iter->path + "/vipdoc/sz/minline";
                    import_func = import_all ? tdx_import_all_min_data : tdx_import_min_data;

                } else {
                    need_import = false;
                }
            }

            if (need_import) {
                std::cout << "导入数据[" << dest_iter->first << "]: " << path << std::endl;
                import_func(db, h5file_dict[dest_iter->first], market, path);
                gettimeofday(&end_time, NULL);
                std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    ///
    /// 更新股票日期信息
    ///
    ///////////////////////////////////////////////////////////////////////

    std::cout << "更新股票日期信息..." << std::endl;
    gettimeofday(&start_time, NULL);
    if (h5file_dict.find("sh_day") != h5file_dict.end()) {
        std::cout << "更新上证股票日期信息..." << std::endl;
        update_all_stock_date(db, h5file_dict["sh_day"], "SH");
    }
    if (h5file_dict.find("sz_day") != h5file_dict.end()) {
        std::cout << "更新深证股票日期信息..." << std::endl;
        update_all_stock_date(db, h5file_dict["sz_day"], "SZ");
    }
    gettimeofday(&end_time, NULL);
    std::cout << mydifftime(end_time, start_time) << "s" << std::endl << std::endl;

    ///////////////////////////////////////////////////////////////////////
    ///
    /// 导入权息数据
    /// （放在最后，因为在更新股票日期时，会将没有K线数据的无效股票删除，这样不用重复导入再删除
    ///
    ///////////////////////////////////////////////////////////////////////

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

    gettimeofday(&total_end_time, NULL);
    double total_time = mydifftime(total_end_time, total_start_time);
    std::cout << "总耗时：" << total_time/60.0 << "m " << total_time << "s" << std::endl << std::endl;

    return 0;
}
