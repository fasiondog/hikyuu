/*
 * importdata.cpp
 *
 *  Created on: 2010-10-26
 *      Author: fasiondog
 */

#include <stdlib.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#else
#include <iconv.h>
#endif

#if defined(BOOST_MSVC)
#include <hdf5.h>
#include <hdf5_hl.h>
#else
#include <hdf5/serial/hdf5.h>
#include <hdf5/serial/hdf5_hl.h>
#endif

#include "sqlite3callback.h"
#include "importdata.h"

//是否使用二分法查找K线文件中第一个大于指定日期的记录，否在则顺序读入文件进行判断
#define USE_FIND_POS 1

//导入全部K线数据，否则只导入stock.db中存在的股票数据
#define IMPORT_ALL 1

namespace bg = boost::gregorian;

/******************************************************************************
 * h5数据表结构全局定义
 *****************************************************************************/
const char *g_h5_data_fieldnames[7] = {"datetime", "openPrice", "highPrice", "lowPrice",
                                  "closePrice", "transAmount", "transCount"};
const size_t g_h5_data_fieldoffset[] = {HOFFSET(H5Record,datetime),
               HOFFSET(H5Record,openPrice), HOFFSET(H5Record,highPrice),
               HOFFSET(H5Record,lowPrice), HOFFSET(H5Record,closePrice),
               HOFFSET(H5Record,transAmount), HOFFSET(H5Record,transCount)};
//const size_t g_h5_data_fieldsize[] = { 8, 4, 4, 4, 4, 8, 8 };

#if defined(WIN32) || defined(_WIN32)
//VC++编译链接时总是提示无法找到H5T_NATIVE_UINT64_g之类的外部变量
//所以此处使用Cpp API中的数据类型定义
const hid_t g_h5_data_fieldtype[] = {H5::PredType::NATIVE_UINT64.getId(),
        H5::PredType::NATIVE_UINT.getId(), H5::PredType::NATIVE_UINT.getId(),
        H5::PredType::NATIVE_UINT.getId(), H5::PredType::NATIVE_UINT.getId(),
        H5::PredType::NATIVE_UINT64.getId(), H5::PredType::NATIVE_UINT64.getId()};
#else
const hid_t g_h5_data_fieldtype[] = {H5T_NATIVE_UINT64,
        H5T_NATIVE_UINT, H5T_NATIVE_UINT,H5T_NATIVE_UINT,
        H5T_NATIVE_UINT, H5T_NATIVE_UINT64, H5T_NATIVE_UINT64};
#endif

/******************************************************************************
 * h5索引表结构全局定义
 *****************************************************************************/
const char *g_h5_index_fieldnames[2] = {"datetime", "start"};
const size_t g_h5_index_fieldoffest[] = {HOFFSET(H5IndexRecord, datetime),
                                    HOFFSET(H5IndexRecord, start)};
//const size_t g_h5_index_fieldsize[] = {8, 8};

#if defined(WIN32) || defined(_WIN32)
const hid_t g_h5_index_fieldtype[] = {H5::PredType::NATIVE_UINT64.getId(),
                                  H5::PredType::NATIVE_UINT64.getId()};
#else
const hid_t g_h5_index_fieldtype[] = {H5T_NATIVE_UINT64, H5T_NATIVE_UINT64};
#endif

// Sqlite3数据库共享指针实例的deleter
class SqliteCloser{
public:
    void operator()(sqlite3 *db){
        if(db){
            std::cout << "Closing Sqlite3 database...";
            sqlite3_close(db);
            std::cout << "Closed!" << std::endl;
        }
    }
};

// H5file的关闭函数，构造shared_ptr<H5::H5File>时使用，用于自动关闭资源
class Hdf5FileCloser{
public:
    void operator()(H5::H5File *h5file){
        if(h5file){
            std::cout << "Closing " << h5file->getFileName() << "...";
            h5file->close();
            std::cout << "Closed!" << std::endl;
        }
    }
};

#if !(defined(WIN32) || defined(_WIN32))
//-----------------------------------------------------------------------------
/**
 *  计算时间差
 *  @param end 终止时间
 *  @param start 起始时间
 *  @return 时间差（end-star),单位秒
 *  @note 仅用于Linux平台
 */
//-----------------------------------------------------------------------------
double mydifftime(struct timeval& end, struct timeval& start){
    int t1 = end.tv_sec - start.tv_sec;
    int t2 = end.tv_usec - start.tv_usec;
    return double(t1)+double(t2)*0.000001;
}

#else /* #if !(defined(WIN32) || defined(_WIN32)) */
void gettimeofday(MY_TIME_VALUE *p_cur_time, MY_TIME_VALUE *) {
    (*p_cur_time) = GetTickCount();
}

double mydifftime(MY_TIME_VALUE& end, MY_TIME_VALUE& start){
    return double((end - start)) / 1000.0;
}
#endif

//-----------------------------------------------------------------------------
// 获取当前日期
//-----------------------------------------------------------------------------
unsigned int get_today_date() {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    boost::gregorian::date date= now.date();
    return (date.year() * 10000 + date.month() * 100 + date.day());
}

//-----------------------------------------------------------------------------
// 控制台进度条
//-----------------------------------------------------------------------------
void progress_bar(int cur, int total) {
    int length = cur * 50 / total;
    printf("\r[");
    for (int i = 0; i < length; ++i) {
        printf("=");
    }
    for (int i = length; i < 50; ++i) {
        printf(" ");
    }
    printf("] %i%%", 100 * cur / total);
    if (cur >= total)
        printf("\n");
    fflush(stdout);
}


//-----------------------------------------------------------------------------
/**
 * 打开指定的sqlite3数据库，如果不存在，则创建新库
 * @param dbname 指定的数据库文件名
 * @return sqlite3数据库指针，如果打开数据库失败返回空指针
 */
//-----------------------------------------------------------------------------
SqlitePtr open_db(const std::string& dbname) {
    sqlite3 *db;
    int rc = sqlite3_open_v2(dbname.c_str(), &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
    if( rc ){
        std::cerr << "Can't open database: " << dbname << std::endl
                << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return SqlitePtr();
    }

    return SqlitePtr(db, SqliteCloser());
}

//-----------------------------------------------------------------------------
/**
 * 根据引入的数据库创建脚本，在数据库中建立相应的表、数据及索引
 * @param db sqlite3数据库指针
 * @param filename 引入的数据库创建脚本
 * @return true 脚本执行成功 \n
 *         false 脚本执行失败
 */
//-----------------------------------------------------------------------------
bool create_database(const SqlitePtr& db, const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file) {
        std::cerr << "[create_database] Can't open sql file: " << filename << std::endl;
        return false;
    }

    //一次性导入所有脚本
    std::stringstream buf(std::stringstream::out);
    std::string temp;
    while (getline(file, temp)) {
        buf << temp;
    }

    char *zErrMsg = 0;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    file.close();
    return true;
}


void dzh_import_stock_name_from_market(const SqlitePtr& db,
        const std::string& market,
        const std::map<std::string, std::string>& new_stock_dict) {

    unsigned int marketid = get_marketid(db, market);

    std::list<StockInfoByMarket> old_stock_list;
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid, code, name, valid from stock where marketid = " << marketid;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(),
            callback_get_stock_info_by_market,
            &old_stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //启动SQL更新事务
    rc = sqlite3_exec(db.get(), "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    std::map<std::string, unsigned int> old_stock_dict;

    std::map<std::string, std::string>::const_iterator new_dict_iter;
    std::list<StockInfoByMarket>::const_iterator old_iter = old_stock_list.begin();
    for (; old_iter != old_stock_list.end(); ++old_iter) {
        old_stock_dict[old_iter->code] = old_iter->stockid;

        new_dict_iter = new_stock_dict.find(old_iter->code);
        if (new_dict_iter == new_stock_dict.end()) {
            //新的代码表中无此股票，则置为无效
            if (old_iter->valid == 1) {
                buf.str("");
                buf << "update stock set valid=0 where stockid=" << old_iter->stockid;
                rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    return;
                }
            }

        } else {
            //股票名称发生变化，更新股票名称;如果原无效，则置为有效
            if (old_iter->name != new_dict_iter->second) {
                buf.str("");
                buf << "update stock set name='" << new_dict_iter->second << "'"
                    << " where stockid=" << old_iter->stockid;
                rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    return;
                }
            }

            if (old_iter->valid == 0) {
                buf.str("");
                buf << "update stock set valid=1, endDate=99999999 where stockid="
                    << old_iter->stockid;
                rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    return;
                }
            }
        }
    }

    CodeTreePtr codeTree = create_CodeTree(db, market);
    if (!codeTree) {
        std::cerr << "[tdx_import_stock_name_from_file] Create CodeTree Failure!\n";
        return;
    }

    unsigned int today = get_today_date();

    for (new_dict_iter = new_stock_dict.begin();
            new_dict_iter != new_stock_dict.end(); ++new_dict_iter) {

        if (old_stock_dict.find(new_dict_iter->first) != old_stock_dict.end()) {
            continue;
        }

        unsigned int stocktype = codeTree->getStockType(new_dict_iter->first);
        if (stocktype == 0)
            continue;

        buf.str("");
        buf << "insert into Stock(marketid, code, name, type, valid, startDate, endDate) "
            << "values ("
            << marketid << ","
            << "'" << new_dict_iter->first << "',"
            << "'" << new_dict_iter->second << "',"
            << stocktype << ","
            << "1,"
            << today << ","
            << "99999999)";
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
    }

    //事务提交
    rc = sqlite3_exec(db.get(), "COMMIT", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}


//-----------------------------------------------------------------------------
/**
 * 从大智慧导入更新每只股票的名称和当前是否有效的标志
 * @detail 如果导入的代码表中不存在对应的代码，则认为该股已失效
 * @param db sqlite数据库
 * @param filename 大智慧的证券代码表文件名
 */
//-----------------------------------------------------------------------------
void dzh_import_stock_name(const SqlitePtr& db, const std::string& dirname) {
    assert(db);

    std::string filename = dirname + "/internet/tcpipdata/init.dat";
    std::ifstream file(filename.c_str(), std::ifstream::binary);
    if( !file ) {
        std::cerr << "[import_stock_name] Can't open file: " << filename << std::endl;
        return;
    }

    std::map<std::string, std::string> sh_market_dict;
    std::map<std::string, std::string> sz_market_dict;
    unsigned int marketid = 0;

    CodeTreePtr codeTree30(new CodeTree);
    codeTree30->addCode("000", 1);
    codeTree30->addCode("39", 2);

    CodeTreePtr codeTree31(new CodeTree);
    codeTree31->addCode("6", 1);
    codeTree31->addCode("9", 1);
    codeTree31->addCode("5", 1);
    codeTree31->addCode("0", 2);
    codeTree31->addCode("1", 2);
    codeTree31->addCode("2", 2);
    codeTree31->addCode("3", 2);

    CodeTreePtr codeTree32(new CodeTree);
    codeTree32->addCode("0", 1);
    codeTree32->addCode("12", 1);
    codeTree32->addCode("10", 2);
    codeTree32->addCode("11", 2);

    char buffer[32];
    char stockname[9];
    char stockcode[7];
    unsigned short stocktype = 0;
    memset(buffer, 0 , 32);
    memset(stockname, 0, 9);
    memset(stockcode, 0, 7);
    file.read(buffer, 28);
    memset(buffer, 0, 32);

    while (file.read(buffer, 32)) {
        memcpy(stockname, buffer, 8);
        memcpy(stockcode, buffer + 8, 6);
        memcpy(&stocktype, buffer + 14, 2);
        memset(buffer, 0, 32);

        marketid = 0;
        switch (stocktype) {
        case 30:
            marketid = codeTree30->getStockType(stockcode);
            break;
        case 31:
            marketid = codeTree31->getStockType(stockcode);
            break;
        case 32:
            marketid = codeTree32->getStockType(stockcode);
            break;
        default:
            ;//do nothing
        }

        if (marketid == 1) {
            sh_market_dict[stockcode] = GBToUTF8(stockname);
        } else if (marketid == 2) {
            sz_market_dict[stockcode] = GBToUTF8(stockname);
        } else {
            ; //忽略
        }
        //std::cout << GBToUTF8(stockname)  << " " << stockcode << " " << stocktype << " " << marketid << std::endl;
    }
    file.close();

    dzh_import_stock_name_from_market(db, "SH", sh_market_dict);
    dzh_import_stock_name_from_market(db, "SZ", sz_market_dict);

#if 0
    //下面的处理中没有包含在新的证劵代码表中存在而在数据库中不存在的情况，考虑到数据库中的股票是根据日线数据
    //生成的，在此种情况下意味着新增的股票还没有日线数据，所以可以暂时忽略不处理
    Output_callback_import_stock_name old_stock_list;
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid, marketid, code from stock";
    int rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_import_stock_name, &old_stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //启动事务处理
    rc = sqlite3_exec(db.get(), "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //处理沪市
    std::vector<SimpleStockRecord>::iterator iter = old_stock_list.sh_stock_list.begin();
    for (; iter != old_stock_list.sh_stock_list.end(); ++iter) {
        buf.str("");
        if (sh_market_dict.count(iter->code) == 0) {
            //在新的股票代码中无法找到，全部置为无效
            buf << "update stock set valid=0 where stockid=" << iter->stockid;
        } else {
            buf << "update stock set name=\'" << sh_market_dict[iter->code]
                << "\', valid=1, endDate=99999999 where stockid=" << iter->stockid;
        }
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            std::cerr << buf.str() << std::endl;
            sqlite3_free(zErrMsg);
            return;
        }
    }

    //处理深市
    iter = old_stock_list.sz_stock_list.begin();
    for (; iter != old_stock_list.sz_stock_list.end(); ++iter) {
        buf.str("");
        if (sz_market_dict.count(iter->code) == 0) {
            //在新的股票代码中无法找到，全部置为无效
            buf << "update stock set valid=0 where stockid=" << iter->stockid;
        } else {
            buf << "update stock set name=\'" << sz_market_dict[iter->code]
                << "\', valid=1, endDate=99999999 where stockid=" << iter->stockid;
        }
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
    }

    //提交更新
    rc = sqlite3_exec(db.get(), "COMMIT", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
#endif
}

//-----------------------------------------------------------------------------
/**
 * GB编码字符串转换至UTF8
 * @param szinput 输入的GB编码字符串
 * @param UTF8编码的字符串
 */
//-----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)
std::string GBToUTF8(char* szinput) {
    wchar_t *strSrc;
    char *szRes;
    std::string nullStr;
    if (!szinput) {
        return nullStr;
    }

    int i = MultiByteToWideChar(CP_ACP, 0, szinput, -1, NULL, 0);
    if (i == 0) {
        return nullStr;
    }
    strSrc = new wchar_t[i+1];
    if (!MultiByteToWideChar(CP_ACP, 0, szinput, -1, strSrc, i)) {
        delete[] strSrc;
        return nullStr;
    }

    i = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, NULL, 0, NULL, NULL);
    if (i == 0) {
        return nullStr;
    }
    szRes = new char[i+1];
    if (!WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, szRes, i, NULL, NULL)) {
        delete[] szRes;
        return nullStr;
    }

    std::string result(szRes);

    delete []strSrc;
    delete []szRes;
    return result;
}

#else
std::string GBToUTF8(char *src_str) {
    std::string result;
    size_t src_len = strlen(src_str);
    size_t buffer_len = src_len * 4 + 1;
    char* buffer= new char[buffer_len];
    memset(buffer, 0, buffer_len);
    buffer_len--;

    iconv_t handle = iconv_open("utf-8", "gbk");
    if (handle == (iconv_t) -1) {
        perror("ICONV:");
        return result;
    }

    char *src = src_str;
    char *dst = buffer;
    size_t rc = iconv(handle, &src, &src_len, &dst, &buffer_len);
    if (rc == (size_t) -1) {
        perror("ICONV:");
        return result;
    }
    if (strlen(buffer) == 0) {
        return result;
    }

    result = std::string(buffer);
    delete [] buffer;
    iconv_close(handle);
    return result;
}
#endif

//-----------------------------------------------------------------------------
/**
 * 判断以unsigned int表示的日期（YYYYMMDD)是否为实际有效日期
 * @param date 需判断的日期
 * @return true or false
 */
//-----------------------------------------------------------------------------
bool invalid_date(unsigned int date) {
    unsigned int year = date / 10000;
    unsigned int month = (date - date / 10000 * 10000) / 100;
    unsigned int day = (date - date / 100 * 100);
    unsigned int month_list[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //闰年，二月是29天
    if (((year % 4 == 0) && (year % 100 != 0)) || ( year % 400 == 0)){
        month_list[2] = 29;
    }

    if (month > 12 || month == 0 || day ==0 || day > month_list[month] ) {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/**
 * 导入股票权息数据（仅支持钱龙）
 * @param db sqlite3数据库
 * @param market 所属市场
 * @param 对应的权息数据所在目录
 */
//-----------------------------------------------------------------------------
void import_stock_weight(const SqlitePtr& db,
                         const std::string& market,
                         const std::string& dir_path) {
    assert(db);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[import_stock_weight] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[import_stock_weight] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    char *zErrMsg = 0;
    int rc;
    std::list<StockCode> stock_list;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid,code from stock where marketid=" << marketid;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stockcode_list, &stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //大数据量插入需要启动事务处理，否则影响性能
    rc = sqlite3_exec(db.get(), "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    int total = stock_list.size();
    int count = 0, cur = 0;
    std::list<StockCode>::const_iterator stk_iter = stock_list.begin();
    for (; stk_iter != stock_list.end(); ++stk_iter) {
        //progress_bar(++cur, total);

        std::string filename = dir_path + "/" + stk_iter->code + ".wgt";
        std::ifstream file(filename, std::ifstream::binary);
        if (!file) {
            continue;
        }

        buf.str("");
        buf << "select date from stkweight where stockid=" << stk_iter->stockid;
        std::map<unsigned int, int> date_map;
        rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_datemap_of_stockweight, &date_map, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            continue;
        }

        int buffer[9];
        memset(buffer, 0 , 36);
        while (file.read((char *)buffer, 36)) {
            unsigned int date = (buffer[0]>>20)*10000 + (((buffer[0]<<12)&4294967295)>>28)*100 + ((buffer[0]&0xffff)>>11);
            if (!invalid_date(date)){
                std::cout << "ignore! " << stk_iter->code << " invalid date: " << date << std::endl;
                continue;
            }
            if (date_map.count(date) == 0) {
                buf.str("");
                buf << "INSERT INTO StkWeight(stockid, date, countAsGift, countForSell, "
                    << "priceForSell, bonus, countOfIncreasement, totalCount, freeCount) VALUES ("
                    << stk_iter->stockid << "," << date << "," << buffer[1] << "," << buffer[2] << "," << buffer[3] << ","
                    << buffer[4] << "," << buffer[5] << "," << buffer[6] << "," << buffer[7] << ")";
                rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_datemap_of_stockweight, &date_map, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    continue;
                }

                count++;

            }
            memset(buffer, 0 , 36);
        }

        file.close();
    }

    rc = sqlite3_exec(db.get(), "COMMIT", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    std::cout << "导入权息数据: " << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 构建指定市场的代码规则树，用于判断证券代码的类型
 * @param db sqlite3数据库
 * @param market 指定的市场
 * @return 该市场的代码规则分析树，如果创建失败，返回空指针
 */
//-----------------------------------------------------------------------------
CodeTreePtr create_CodeTree(const SqlitePtr& db, const std::string& market) {
    assert(db);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        return CodeTreePtr();
    }

    CodeTreePtr result(new CodeTree);
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select codepre, type from CodeRuleType where marketid=" << marketid;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_codeprefix, &result, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return CodeTreePtr();
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 根据market标识（‘SH’，‘SZ）获取相应的marketid
 * @param db slite3数据库指针
 * @param market 数据库标识
 * @return 对应的marketid，如果为0，表示无效
 */
//-----------------------------------------------------------------------------
unsigned int get_marketid(const SqlitePtr& db, const std::string& market) {
    assert(db);
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select marketid from market where market=\'" << market << "\'";
    unsigned int result = 0;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_marketid, &result, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 0;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 根据marketid和证券代码获取相应的stockid
 * @param db slite3数据库指针
 * @param marketid 市场标识
 * @param code 证券代码
 * @return 对应的stockid，如果为0，表示无效
 */
//-----------------------------------------------------------------------------
unsigned int get_stockid(const SqlitePtr& db, unsigned int marketid, const std::string& code) {
    assert(db);
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid from stock where marketid=" << marketid << " and code=\'" << code << "\'";
    unsigned int result = 0;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_marketid, &result, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 0;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 根据marketid和证券代码获取相应的stock类型
 * @param db slite3数据库指针
 * @param marketid 市场标识
 * @param code 证券代码
 * @return 对应的stock类型
 */
//-----------------------------------------------------------------------------
unsigned int get_stock_type(const SqlitePtr& db,
        unsigned int marketid, const std::string& code) {
    assert(db);
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select type from stock where marketid=" << marketid << " and code=\'" << code << "\'";
    unsigned int result = 0;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stock_type, &result, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 0;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 获取HDF5表数据结构定义
 * @param HDF5数据结构类型定义
 */
//-----------------------------------------------------------------------------
H5CompTypePtr h5_get_data_type() {
    H5CompTypePtr result(new H5::CompType(sizeof(H5Record)));
    result->insertMember("datetime",HOFFSET(H5Record,datetime),H5::PredType::NATIVE_UINT64);
    result->insertMember("openPrice",HOFFSET(H5Record,openPrice),H5::PredType::NATIVE_UINT);
    result->insertMember("highPrice",HOFFSET(H5Record,highPrice),H5::PredType::NATIVE_UINT);
    result->insertMember("lowPrice",HOFFSET(H5Record,lowPrice),H5::PredType::NATIVE_UINT);
    result->insertMember("closePrice",HOFFSET(H5Record,closePrice),H5::PredType::NATIVE_UINT);
    result->insertMember("transAmount",HOFFSET(H5Record,transAmount),H5::PredType::NATIVE_UINT64);
    result->insertMember("transCount",HOFFSET(H5Record,transCount),H5::PredType::NATIVE_UINT64);
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 获取HDF5表索引结构定义
 * @param HDF5索引结构类型定义
 */
//-----------------------------------------------------------------------------
H5CompTypePtr h5_get_index_type() {
    H5CompTypePtr result(new H5::CompType(sizeof(H5IndexRecord)));
    result->insertMember("datetime",HOFFSET(H5IndexRecord,datetime),H5::PredType::NATIVE_UINT64);
    result->insertMember("start",HOFFSET(H5IndexRecord,start),H5::PredType::NATIVE_UINT64);
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 获取HDF5表记录总数
 * @param dataset 指定的HDF5数据集
 * @return 记录总数
 */
//-----------------------------------------------------------------------------
hssize_t h5_get_nrecords(const H5::DataSet& dataset) {
    H5::DataSpace dataspace = dataset.getSpace();
    hssize_t result = dataspace.getSelectNpoints();
    dataspace.close();
    return result;
}

//-----------------------------------------------------------------------------
/**
 *  读取HDF5文件记录
 *  @param H5::DataSet& dataset 指定的数据集
 *  @param H5::CompType& h5Type 数量类型
 *  @param hsize_t start 起始数据记录号
 *  @param hsize_t nrecords 欲读取的数据记录数
 *  @param [out] *data 读取的数据结果，需由调用者自己负责申请内存空间
 *  @note *data缓存空间由用户自行申请，必须谨慎
 */
 //-----------------------------------------------------------------------------
void h5_read_records(H5::DataSet& dataset, H5::CompType h5Type, hsize_t start, hsize_t nrecords, void *data){
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t offset[1];
    hsize_t count[1];
    offset[0] = start;
    count[0] = nrecords;
    H5::DataSpace memspace(1,count);
    dataspace.selectHyperslab(H5S_SELECT_SET,count,offset);
    dataset.read(data, h5Type, memspace, dataspace);
    memspace.close();
    dataspace.close();
    return;
}

//-----------------------------------------------------------------------------
/**
 * 在指定的HDF5数据集中增加新的记录
 *  @param H5::DataSet& dataset 指定的数据集
 *  @param H5::CompType& h5Type 数量类型
 *  @param hsize_t nrecords 欲写入的数据记录总数
 *  @param *data 待写入的数据记录集
 */
//-----------------------------------------------------------------------------
void h5_append_records(H5::DataSet& dataset, H5::CompType h5Type, hsize_t nrecords, void *data){
    hsize_t offset[1];
    hsize_t count[1];
    hsize_t dims[1];
    offset[0] = h5_get_nrecords(dataset);
    count[0] = nrecords;
    dims[0] = offset[0] + count[0];
    dataset.extend(dims);
    H5::DataSpace memspace(1,count);
    H5::DataSpace dataspace = dataset.getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET,count,offset);
    dataset.write(data, h5Type, memspace, dataspace);
    dataspace.close();
    memspace.close();
    return;
}

//-----------------------------------------------------------------------------
/**
 * 打开指定HDF5文件，如果不存在，则创建新的HDF5文件
 * @param filename 指定的HDF5文件名
 * @param HDF5文件指针，如果打开失败，返回空指针
 * @note 在打开过程中，同时初始化全局变量: g_h5_data_type、g_h5_index_type
 */
//-----------------------------------------------------------------------------
H5FilePtr h5_open_file(const std::string& filename) {
    H5FilePtr result;
    H5FilePtr null_result;
    if (access(filename.c_str(), 0)) {
        //指定文件不存在
        try {
           result = H5FilePtr(new H5::H5File(filename, H5F_ACC_TRUNC),Hdf5FileCloser());
        } catch(H5::FileIException&) {
           std::cerr << "[h5_open_file] Can't open " << filename << std::endl;
           return null_result;
        }
    } else {
        //指定的文件存在
        if (!H5::H5File::isHdf5(filename)) {
            std::cerr << "[h5_open_file] " << filename << " is not in HDF5 format!\n";
            return null_result;
        }
        try {
            result = H5FilePtr(new H5::H5File(filename, H5F_ACC_RDWR),Hdf5FileCloser());
        } catch (H5::FileIException&) {
            std::cerr << "[h5_open_file] Can't open " << filename << std::endl;
            return null_result;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/**
 * 获取指定的H5::Group，如果不存在，则创建
 * @param h5file HDF5文件实例
 * @param name group名称
 * @return H5::Group
 */
//-----------------------------------------------------------------------------
H5::Group h5_get_group(const H5FilePtr& h5file, const std::string& name) {
    assert(h5file);
    H5::Group result;
    try {
        result = h5file->openGroup(name);
    //openGroup如果不存在指定的group时，不知抛出什么异常（不是GroupIException也不是
    //FileIException），参考手册中描述不清，所以此处捕获所有异常
    } catch (...) {
        result = h5file->createGroup(name);
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 判断指定名称的数据表是否存在
 * @param group 指定的HDF5 Group路径
 * @param name 指定的股票表名（market+code，如 SH000001，SZ000001）
 * @return 对应的K线数据集
 */
//-----------------------------------------------------------------------------
bool h5_is_exist_data_table(const H5::Group& group, const std::string& name) {
    hid_t dataset_id = H5Dopen(group.getId(), name.c_str());
    return dataset_id < 0 ? false : true;
}

//-----------------------------------------------------------------------------
/**
 * 打开指定股票的K线数据表，如果不存在，则创建新表并返回
 * @param group 指定的HDF5 Group路径
 * @param name 指定的股票表名（market+code，如 SH000001，SZ000001）
 * @return 对应的K线数据集
 * @note 由于HDF5 C++ API不支持创建空数据集，直接使用C API实现
 */
//-----------------------------------------------------------------------------
H5::DataSet h5_get_data_table(const H5::Group& group, const std::string& name) {
    hid_t loc_id = group.getId();
    hid_t dataset_id = H5Dopen(loc_id, name.c_str());

    //指定的数据集不存在
    if (dataset_id < 0) {
        herr_t err = H5TBmake_table(name.c_str(), loc_id, name.c_str(), 7, 0,
                sizeof(H5Record), g_h5_data_fieldnames, g_h5_data_fieldoffset,
                g_h5_data_fieldtype, 240, NULL, 9, NULL);
        if (err < 0) {
            std::cout << "[h5_get_table] Error!" << std::endl;
        } else {
            dataset_id = H5Dopen(loc_id, name.c_str());
        }
    }

    return H5::DataSet(dataset_id);
}

//-----------------------------------------------------------------------------
/**
 * 打开指定股票的K线索引表，如果不存在，则创建新表并返回
 * @param group 指定的HDF5 Group路径
 * @param name 指定的股票表名（market+code，如 SH000001，SZ000001）
 * @return 对应的K线数据集
 * @note 由于HDF5 C++ API不支持创建空数据集，直接使用C API实现
 */
//-----------------------------------------------------------------------------
H5::DataSet h5_get_index_table(const H5::Group& group, const std::string& name) {
    hid_t loc_id = group.getId();
    hid_t dataset_id = H5Dopen(loc_id, name.c_str());

    //指定的数据集不存在
    if (dataset_id < 0) {
        herr_t err = H5TBmake_table(name.c_str(), loc_id, name.c_str(), 2, 0,
                sizeof(H5IndexRecord), g_h5_index_fieldnames, g_h5_index_fieldoffest,
                g_h5_index_fieldtype, 160, NULL, 9, NULL);
        if (err < 0) {
            std::cout << "[h5_get_index_table] Error!" << std::endl;
        } else {
            dataset_id = H5Dopen(loc_id, name.c_str());
        }
    }

    return H5::DataSet(dataset_id);
}

//-----------------------------------------------------------------------------
/**
 * 判断钱龙K线数据是否有效（即是否存在错误，如最高价小于最低价)
 * @param data 钱龙K线数据
 * @note 不包含对非法日期的判断
 */
//-----------------------------------------------------------------------------
bool invalid_QianLongData(const QianLongData& data) {
    //最高价小于最低价，无效
    if (data.low > data.high || data.open > data.high || data.close > data.high
            || data.open < data.low || data.close < data.low) {
        return false;
    }

    //价格数据中存在为0的数据 或 成交量与成交金额同时为零，认为无效
    if ((data.amount == 0 && data.count == 0)
            || data.open == 0 || data.high == 0
            || data.low == 0 || data.close == 0) {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/**
 * 获取所属周一的日期
 * @param olddate YYYYMMDDhhmm 标识的日期
 * @note 认为周一是一周中的第一天，周日是一周中的最后一天
 */
//-----------------------------------------------------------------------------
unsigned long long get_week_date(unsigned long long olddate) {
    unsigned long long y = olddate/100000000LL;
    unsigned long long m = olddate/1000000LL - y*100LL;
    unsigned long long d = olddate/10000LL - (y*10000+m*100LL);
    bg::date tempdate(y, m, d);
    long day = tempdate.day_of_week();
    day = day ? day - 1 : 6;
    bg::date tempweekdate = tempdate - boost::gregorian::date_duration(day);
    return (unsigned long long)tempweekdate.year() * 100000000LL +
            (unsigned long long)tempweekdate.month() * 1000000LL +
            (unsigned long long)tempweekdate.day() * 10000LL;
}

//-----------------------------------------------------------------------------
/**
 * 获取所属月份的起始日期
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_month_date(unsigned long long olddate) {
    unsigned long long y = olddate/100000000LL;
    unsigned long long m = olddate/1000000LL - y*100LL;
    return y * 100000000LL + m * 1000000LL + 10000LL;
}

//-----------------------------------------------------------------------------
/**
 * 获取所属季度的起始日期
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_quarter_date(unsigned long long olddate) {
    unsigned long long y = olddate/100000000LL;
    unsigned long long m = olddate/1000000LL - y*100LL;
    unsigned long long new_m;
    if (m >= 1 && m < 4) {
        new_m = 1;
    } else if (m >=4 && m < 7) {
        new_m = 2;
    } else if (m >= 7 && m < 10) {
        new_m = 3;
    } else {
        new_m = 4;
    }
    return y * 100000000LL + new_m * 1000000LL + 10000LL;
}

//-----------------------------------------------------------------------------
/**
 * 获取所属半年的起始日期
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_halfyear_date(unsigned long long olddate) {
    unsigned long long y = olddate/100000000LL;
    unsigned long long m = olddate/1000000LL - y*100LL;
    unsigned long long new_m = m > 6 ? 7 : 1;
    return y * 100000000LL + new_m * 1000000LL + 10000LL;
}

//-----------------------------------------------------------------------------
/**
 * 获取所属年的起始日期
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_year_date(unsigned long long olddate) {
    unsigned long long y = olddate/100000000LL;
    return y * 100000000LL + 1010000LL;
}

//-----------------------------------------------------------------------------
/**
 * 获取15分钟线的起始时间
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_min15_date(unsigned long long olddate) {
    unsigned long long date = olddate/10000LL * 10000LL;
    unsigned long long hhmm = olddate - date;
    unsigned long long result;
    if (hhmm <= 945LL) {
        result = date + 945LL;
    } else if (hhmm <= 1000LL) {
        result = date + 1000LL;
    } else if (hhmm <= 1015LL) {
        result = date + 1015LL;
    } else if (hhmm <= 1030LL) {
        result = date + 1030LL;
    } else if (hhmm <= 1045LL) {
        result = date + 1045LL;
    } else if (hhmm <= 1100LL) {
        result = date + 1100LL;
    } else if (hhmm <= 1115LL) {
        result = date + 1115LL;
    } else if (hhmm <= 1130LL) {
        result = date + 1130LL;
    } else if (hhmm <= 1315LL) {
        result = date + 1315LL;
    } else if (hhmm <= 1330LL) {
        result = date + 1330LL;
    } else if (hhmm <=1345LL) {
        result = date + 1345LL;
    } else if (hhmm <= 1400LL) {
        result = date + 1400LL;
    } else if (hhmm <= 1415LL) {
        result = date + 1415LL;
    } else if (hhmm <= 1430LL) {
        result = date + 1430LL;
    } else if (hhmm <= 1445LL) {
        result = date + 1445LL;
    } else {
        result = date + 1500LL;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 获取30分钟线的起始时间
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_min30_date(unsigned long long olddate) {
    unsigned long long date = olddate/10000LL * 10000LL;
    unsigned long long hhmm = olddate - date;
    unsigned long long result;
    if (hhmm <= 1000LL) {
        result = date + 1000LL;
    } else if (hhmm <= 1030LL) {
        result = date + 1030LL;
    } else if (hhmm <= 1100LL) {
        result = date + 1100LL;
    } else if (hhmm <= 1130LL) {
        result = date + 1130LL;
    } else if (hhmm <= 1330LL) {
        result = date + 1330LL;
    } else if (hhmm <= 1400LL) {
        result = date + 1400LL;
    } else if (hhmm <= 1430LL) {
        result = date + 1430LL;
    } else {
        result = date + 1500LL;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 获取60分钟线的起始时间
 * @param olddate YYYYMMDDhhmm 标识的日期
 */
//-----------------------------------------------------------------------------
unsigned long long get_min60_date(unsigned long long olddate) {
    unsigned long long date = olddate/10000LL * 10000LL;
    unsigned long long hhmm = olddate - date;
    unsigned long long result;
    if (hhmm <= 1030LL) {
        result = date + 1030LL;
    } else if (hhmm <= 1130LL) {
        result = date + 1130LL;
    } else if (hhmm <= 1400LL) {
        result = date + 1400LL;
    } else {
        result = date + 1500LL;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 根据索引类型，获取索引对应的日期
 * @param olddate 原始日期
 * @param ix_type 索引类型
 * @note 仅供update_h5_index函数调用
 */
//-----------------------------------------------------------------------------
unsigned long long get_new_date(unsigned long long olddate, H5_INDEX_TYPE ix_type) {
    unsigned long long result;
    switch (ix_type) {
    case INDEX_WEEK:
        result = get_week_date(olddate);
        break;
    case INDEX_MONTH:
        result = get_month_date(olddate);
        break;
    case INDEX_QUARTER:
        result = get_quarter_date(olddate);
        break;
    case INDEX_HALFYEAR:
        result = get_halfyear_date(olddate);
        break;
    case INDEX_YEAR:
        result = get_year_date(olddate);
        break;
    case INDEX_MIN15:
        result = get_min15_date(olddate);
        break;
    case INDEX_MIN30:
        result = get_min30_date(olddate);
        break;
    case INDEX_MIN60:
        result = get_min60_date(olddate);
        break;
    default:
        std::cerr << "[get_new_date] Can't know H5_INDEX_TYPE! (" << ix_type << ")\n";
        result = 0;
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 根据索引类型获取相应索引表的名称
 * @param ix_type 指定的索引类型
 * @return 对应的索引表名称
 * @note 仅供update_h5_index函数调用
 */
//-----------------------------------------------------------------------------
std::string get_h5_index_table_name(H5_INDEX_TYPE ix_type) {
    std::string result;
    switch (ix_type) {
    case INDEX_WEEK:
        result = "/week";
        break;
    case INDEX_MONTH:
        result = "/month";
        break;
    case INDEX_QUARTER:
        result = "/quarter";
        break;
    case INDEX_HALFYEAR:
        result = "/halfyear";
        break;
    case INDEX_YEAR:
        result = "/year";
        break;
    case INDEX_MIN15:
        result = "/min15";
        break;
    case INDEX_MIN30:
        result = "/min30";
        break;
    case INDEX_MIN60:
        result = "/min60";
        break;
    default:
        std::cerr << "[get_index_table_name] Can't know H5_INDEX_TYPE! (" << ix_type << ")\n";
    }
    return result;
}

//-----------------------------------------------------------------------------
/**
 * 更新H5相关索引
 * @param h5file 指定的H5文件指针
 * @param table_name 指定的表名
 * @param ix_type 指定的索引类型
 */
//-----------------------------------------------------------------------------
void update_h5_index(const H5FilePtr& h5file, const std::string& table_name, H5_INDEX_TYPE ix_type) {
    H5::Group h5_data_group = h5_get_group(h5file, "/data");
    if (!h5_is_exist_data_table(h5_data_group, table_name))
        return;

    H5::DataSet h5_data_table = h5_get_data_table(h5_data_group, table_name);

    H5::Group h5_index_group = h5_get_group(h5file, get_h5_index_table_name(ix_type));
    H5::DataSet h5_index_table = h5_get_index_table(h5_index_group, table_name);

    //放在读取索引表之后判断，即使没有初始数据，也需要创建相应的索引表
    hsize_t data_total = h5_get_nrecords(h5_data_table);
    if (data_total == 0) {
        return;
    }

    H5CompTypePtr h5_data_type = h5_get_data_type();
    H5CompTypePtr h5_index_type = h5_get_index_type();

    H5Record data_record;
    H5IndexRecord index_record;
    unsigned long long pre_index_date;
    hsize_t start_ix = 0;
    hssize_t index_total = h5_get_nrecords(h5_index_table);
    if (index_total) {
        h5_read_records(h5_index_table, *h5_index_type, index_total-1, 1, &index_record);
        h5_read_records(h5_data_table, *h5_data_type, data_total-1, 1, &data_record);
        if (index_record.datetime == get_new_date(data_record.datetime, ix_type)) {
            return;
        }
        start_ix = index_record.start + 1;
        pre_index_date = index_record.datetime;
    } else {
        start_ix = 0;
        h5_read_records(h5_data_table, *h5_data_type, 0, 1, &data_record);
        pre_index_date = get_new_date(data_record.datetime, ix_type);
        index_record.datetime = pre_index_date;
        index_record.start = 0;
        h5_append_records(h5_index_table, *h5_index_type, 1, &index_record);
    }

    if (data_total <= start_ix) {
        return;
    }
    hsize_t buffer_len = data_total - start_ix;
    H5Record *data_buffer = new H5Record[buffer_len];
    H5IndexRecord *index_buffer = new H5IndexRecord[buffer_len];
    h5_read_records(h5_data_table, *h5_data_type, start_ix, buffer_len, data_buffer);
    hsize_t index_i = 0;
    hsize_t index = start_ix;
    for (hsize_t i = 0; i < buffer_len; i++) {
        unsigned long long cur_index_date = get_new_date(data_buffer[i].datetime, ix_type);
        if (cur_index_date != pre_index_date) {
            index_buffer[index_i].datetime = cur_index_date;
            index_buffer[index_i].start = index;
            index_i++;
            pre_index_date = cur_index_date;
        }
        index++;
    }

    if (index_i > 0 ) {
        h5_append_records(h5_index_table, *h5_index_type, index_i, index_buffer);
    }

    delete [] data_buffer;
    delete [] index_buffer;
}


/*
 * 获取大智慧日线文件记录数
 */
int dzh_get_day_data_count(const std::string& filename) {
    if (!fs::exists(filename))
        return 0;

    struct stat statbuf;
    stat(filename.c_str(), &statbuf);
    int size = statbuf.st_size;
    return size / sizeof(QianLongData);
}

/*
 * 获取大智慧日线文件指定记录的日期
 */
unsigned int dzh_get_day_data_date(std::ifstream& file, int pos) {
    file.seekg(pos * sizeof(QianLongData), file.beg);
    unsigned int date = 0;
    file.read((char *)&date, sizeof(date));
    return date;
}


/*
 * 选择大智慧日线文件中第一个大于等于lastdate的记录位置
 */
int dzh_day_data_find_pos(std::ifstream& file, unsigned int last_date) {
    file.seekg(0, file.end);
    int total = file.tellg() / sizeof(QianLongData);

    int low = 0, high = total - 1;
    int mid = high / 2;
    while (mid <= high) {
        unsigned int cur_date = dzh_get_day_data_date(file, low);
        if (cur_date > last_date) {
            mid = low;
            break;
        }

        cur_date = dzh_get_day_data_date(file, high);
        if (cur_date <= last_date) {
            mid = high + 1;
            break;
        }

        cur_date = dzh_get_day_data_date(file, mid);
        if (cur_date <= last_date) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }

        mid = (low + high) / 2;
    }

    return mid;
}

//-----------------------------------------------------------------------------
/**
 * 从钱龙或大智慧数据文件中导入日线数据到目标HDF5文件
 * @param file_name 钱龙或大智慧数据文件名
 * @param h5file 目标HDF5文件
 * @param table_name 目标表名
 * @return 成功导入的数据量
 */
//-----------------------------------------------------------------------------
int dzh_import_day_data_from_file(const std::string& file_name,
                                  const H5FilePtr& h5file,
                                  const std::string& table_name) {
    int total = dzh_get_day_data_count(file_name);
    if (total == 0)
        return 0;

    std::ifstream file(file_name.c_str(), std::ifstream::binary);
    if (!file)
        return 0;

    H5::DataSet h5_table;
    unsigned int last_date = 0;
    H5CompTypePtr h5_data_type = h5_get_data_type();

    H5::Group h5_group = h5_get_group(h5file, "/data");
    bool is_exist_data_table = h5_is_exist_data_table(h5_group, table_name);
    if (is_exist_data_table) {
        h5_table = h5_get_data_table(h5_group, table_name);
        hssize_t nrecords = h5_get_nrecords(h5_table);
        if (nrecords > 0) {
            H5Record last_record;
            h5_read_records(h5_table, *h5_data_type, nrecords-1, 1, &last_record);
            last_date = (unsigned int)(last_record.datetime / 10000);
        }
    }

#if USE_FIND_POS
    //int total = dzh_get_day_data_count(file);
    int pos = dzh_day_data_find_pos(file, last_date);
    if (pos >= total) {
        file.close();
        h5_group.close();
        return 0;
    }

    file.seekg(pos * sizeof(QianLongData), file.beg);
#endif

    std::vector<H5Record> h5_buffer;
    QianLongData day_data;
    memset(&day_data, 0, sizeof(QianLongData));
    while (file.read((char *)&day_data, sizeof(QianLongData))){

#if !USE_FIND_POS
        //如果日期小于或等于最后日期，则跳过
        if (day_data.date <= last_date) {
            //std::cout << "invalid date(date <= last_date): " << last_record.datetime << " "
            //		<< last_date << " " << day_data.date << " " << table_name << std::endl;
            continue;
        }
#endif

        //如果日期无效，则跳过
        if (!invalid_date(day_data.date)) {
            //std::cout << "!invalid_date(date): " << day_data.date << " " << table_name << std::endl;
            continue;
        }
        //日线数据无效，则跳过
        if (!invalid_QianLongData(day_data)) {
            //std::cout << "!invalid_QianLongData(day_data): " << table_name << " " << day_data.date << std::endl;
            continue;
        }

        H5Record h5_record;
        h5_record.datetime = (unsigned long long)day_data.date * 10000;
        h5_record.openPrice = day_data.open;
        h5_record.highPrice = day_data.high;
        h5_record.lowPrice = day_data.low;
        h5_record.closePrice = day_data.close;
        h5_record.transAmount = day_data.amount;
        h5_record.transCount = day_data.count;

        h5_buffer.push_back(h5_record);
        //h5_append_records(h5_table, *h5_data_type, 1, &h5_record);

        memset(&day_data, 0 , sizeof(QianLongData));
        last_date = day_data.date;
    }

    if (h5_buffer.size() > 0) {
        if (!is_exist_data_table)
            h5_table = h5_get_data_table(h5_group, table_name);

        h5_append_records(h5_table, *h5_data_type,
                h5_buffer.size(), h5_buffer.data());
    }

    file.close();
    h5_group.close();
    return h5_buffer.size();
}

/*
 * 获取大智慧5分钟线文件记录数
 */
int dzh_get_min_data_count(const std::string& filename) {
    if (!fs::exists(filename))
        return 0;

    struct stat statbuf;
    stat(filename.c_str(), &statbuf);
    int size = statbuf.st_size;
    return size / sizeof(QianLongData);
}

unsigned long long dzh_min_data_trans_date(unsigned int date)
{
    unsigned int tempval = 0xFFFFFFFF;
    unsigned int year, month, day, hh, mm;
    year = date >> 20;
    month = ((date << 12) & tempval) >> 28;
    day = ((date << 16) & tempval) >> 27;
    hh = ((date << 21) & tempval) >> 27;
    mm = ((date << 26) & tempval) >> 26;
    return  (unsigned long long)year*100000000LL
          + (unsigned long long)month*1000000LL
          + (unsigned long long)day*10000LL
          + (unsigned long long)hh*100LL
          + (unsigned long long)mm;
}

/*
 * 获取大智慧5分钟线文件指定记录的日期
 */
unsigned long long dzh_get_min_data_date(std::ifstream& file, int pos) {
    file.seekg(pos * sizeof(QianLongData), file.beg);
    unsigned int date = 0;
    file.read((char *)&date, sizeof(date));
    return dzh_min_data_trans_date(date);
}


/*
 * 选择大智慧5分钟线文件中第一个大于等于lastdate的记录位置
 */
int dzh_min_data_find_pos(std::ifstream& file, unsigned long long last_date) {
    file.seekg(0, file.end);
    int pos = file.tellg();
    int total = pos / sizeof(QianLongData);

    int low = 0, high = total - 1;
    int mid = high / 2;
    while (mid <= high) {
        unsigned long long cur_date = dzh_get_min_data_date(file, low);
        if (cur_date > last_date) {
            mid = low;
            break;
        }

        cur_date = dzh_get_min_data_date(file, high);
        if (cur_date <= last_date) {
            mid = high + 1;
            break;
        }

        cur_date = dzh_get_min_data_date(file, mid);
        if (cur_date <= last_date) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }

        mid = (low + high) / 2;
    }

    return mid;
}


//-----------------------------------------------------------------------------
/**
 * 从钱龙或大智慧数据文件中导入5分钟线数据到目标HDF5文件
 * @param file_name 钱龙或大智慧数据文件名
 * @param h5file 目标HDF5文件
 * @param table_name 目标表名
 * @return 返回成功导入的数据量
 */
//-----------------------------------------------------------------------------
int dzh_import_min5_data_from_file(const std::string& file_name, const H5FilePtr& h5file,
                               const std::string& table_name) {
    int total = dzh_get_min_data_count(file_name);
    if (total == 0) {
        return 0;
    }

    std::ifstream file(file_name.c_str(), std::ifstream::binary);
    if (!file)
        return 0;

    H5::DataSet h5_table;
    H5CompTypePtr h5_data_type = h5_get_data_type();
    unsigned long long last_datetime = 0;

    H5::Group h5_group = h5_get_group(h5file, "/data");
    bool is_exist_data_table = h5_is_exist_data_table(h5_group, table_name);
    if (is_exist_data_table) {
        h5_table = h5_get_data_table(h5_group, table_name);
        hssize_t nrecords = h5_get_nrecords(h5_table);
        H5Record last_record;
        if (nrecords > 0) {
            h5_read_records(h5_table, *h5_data_type, nrecords-1, 1, &last_record);
            last_datetime = last_record.datetime;
        }
    }

#if USE_FIND_POS
    //int total = dzh_get_min_data_count(file);
    int pos = dzh_min_data_find_pos(file, last_datetime);
    if (pos >= total) {
        file.close();
        h5_group.close();
        return 0;
    }

    file.seekg(pos * sizeof(QianLongData), file.beg);
#endif

    std::vector<H5Record> h5_buffer;
    QianLongData data;
    unsigned int tempval = 0xFFFFFFFF;
    memset(&data, 0, sizeof(QianLongData));
    while (file.read((char *)&data, sizeof(QianLongData))){

        unsigned long long cur_datetime = dzh_min_data_trans_date(data.date);

#if !USE_FIND_POS
        //如果日期小于或等于最后日期，则跳过
        if (cur_datetime <= last_datetime) {
            //std::cout << "invalid date(date <= last_date): " << day_data.date << " " << table_name << std::endl;
            continue;
        }
#endif
        //如果日期无效，则跳过
        unsigned int cur_date = (unsigned int)(cur_datetime / 10000LL);
        if (!invalid_date(cur_date)) {
            std::cout << "!invalid_date(date): " << cur_datetime << " " << table_name << std::endl;
            continue;
        }
        //日线数据无效，则跳过
        if (!invalid_QianLongData(data)) {
            //std::cout << "!invalid_QianLongData(day_data): " << table_name << " " << day_data.date << std::endl;
            continue;
        }

        H5Record h5_record;
        h5_record.datetime = cur_datetime;
        h5_record.openPrice = data.open;
        h5_record.highPrice = data.high;
        h5_record.lowPrice = data.low;
        h5_record.closePrice = data.close;
        h5_record.transAmount = data.amount;
        h5_record.transCount = data.count;

        h5_buffer.push_back(h5_record);
        //h5_append_records(h5_table, *h5_data_type, 1, &h5_record);

        memset(&data, 0 , sizeof(QianLongData));
        last_datetime = cur_datetime;
    }

    if (h5_buffer.size() > 0) {
        if (!is_exist_data_table)
            h5_table = h5_get_data_table(h5_group, table_name);

        h5_append_records(h5_table, *h5_data_type,
                          h5_buffer.size(), h5_buffer.data());
    }

    file.close();
    h5_group.close();
    return h5_buffer.size();
}

//-----------------------------------------------------------------------------
/**
 * 导入日线数据
 * @param db sqlite3数据库
 * @param h5 日线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void dzh_import_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[import_day_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[import_day_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    char *zErrMsg = 0;
    int rc;
    std::list<StockCode> stock_list;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid,code from stock where marketid=" << marketid;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stockcode_list, &stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    int total = stock_list.size();
    int count = 0, cur = 0;
    std::list<StockCode>::const_iterator stk_iter = stock_list.begin();
    for (; stk_iter != stock_list.end(); ++stk_iter) {
        progress_bar(++cur, total);
        std::string filename = dir_path.string()
                             + "/" + stk_iter->code + ".day";
        std::string table_name = market + stk_iter->code;

        //std::cout << filename << std::endl;
        count += dzh_import_day_data_from_file(filename, h5, table_name);

        update_h5_index(h5, table_name, INDEX_WEEK);
        update_h5_index(h5, table_name, INDEX_MONTH);
        update_h5_index(h5, table_name, INDEX_QUARTER);
        update_h5_index(h5, table_name, INDEX_HALFYEAR);
        update_h5_index(h5, table_name, INDEX_YEAR);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 导入大智慧所有日线数据，不论该股票是否在数据库中存在
 * @param db sqlite3数据库
 * @param h5 日线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void dzh_import_all_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[import_all_day_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[import_all_day_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    if (!fs::is_directory(dir_path)) {
        std::cerr << "[import_all_day_data] dir_path(" << dir_path << ") is not a directory!\n";
        return;
    }

    fs::directory_iterator end_iter;
    fs::directory_iterator count_itr(dir_path);
    int total = 0;
    for (; count_itr != end_iter; ++count_itr) {
        total++;
    }

    int count = 0, cur = 0;
    fs::directory_iterator dir_itr(dir_path);
    for (; dir_itr != end_iter; ++dir_itr) {
        progress_bar(++cur, total);
        if (fs::is_directory( dir_itr->status() ) ) {
            continue;
        }

        std::string code = fs::basename(dir_itr->path());
        std::string table_name = market + code;

        count += dzh_import_day_data_from_file(dir_itr->path().string(), h5, table_name);

        update_h5_index(h5, table_name, INDEX_WEEK);
        update_h5_index(h5, table_name, INDEX_MONTH);
        update_h5_index(h5, table_name, INDEX_QUARTER);
        update_h5_index(h5, table_name, INDEX_HALFYEAR);
        update_h5_index(h5, table_name, INDEX_YEAR);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 导入5分钟线数据
 * @param db sqlite3数据库
 * @param h5 5分种线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void dzh_import_min5_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[import_min5_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[import_min5_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    char *zErrMsg = 0;
    int rc;
    std::list<StockCode> stock_list;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid,code from stock where marketid=" << marketid;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stockcode_list, &stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    int total = stock_list.size();
    int count = 0, cur = 0;
    std::list<StockCode>::const_iterator stk_iter = stock_list.begin();
    for (; stk_iter != stock_list.end(); ++stk_iter) {
        progress_bar(++cur, total);
        std::string filename = dir_path.string()
                             + "/" + stk_iter->code + ".NMN";
        std::string table_name = market + stk_iter->code;

        count += dzh_import_min5_data_from_file(filename, h5, table_name);

        update_h5_index(h5, table_name, INDEX_MIN15);
        update_h5_index(h5, table_name, INDEX_MIN30);
        update_h5_index(h5, table_name, INDEX_MIN60);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 导入大智慧所有5分钟线数据，不论股票是否在数据库中存在
 * @param db sqlite3数据库
 * @param h5 5分种线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void dzh_import_all_min5_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[import_min5_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[import_min5_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    if (!fs::is_directory(dir_path)) {
        std::cerr << "[import_min5_data] dir_path(" << dir_path << ") is not a directory!\n";
        return;
    }

    fs::directory_iterator end_iter;
    fs::directory_iterator count_itr(dir_path);
    int total = 0;
    for (; count_itr != end_iter; ++count_itr) {
        total++;
    }

    int count = 0, cur = 0;
    fs::directory_iterator dir_itr(dir_path);
    for (; dir_itr != end_iter; ++dir_itr) {
        progress_bar(++cur, total);
        if (fs::is_directory( dir_itr->status() ) ) {
            continue;
        }

        std::string code = fs::basename(dir_itr->path());
        std::string table_name = market + code;

        count += dzh_import_min5_data_from_file(dir_itr->path().string(), h5, table_name);

        update_h5_index(h5, table_name, INDEX_MIN15);
        update_h5_index(h5, table_name, INDEX_MIN30);
        update_h5_index(h5, table_name, INDEX_MIN60);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 将字符串信息，转换为H5Record
 * @param [in] str 输入的字符串
 * @param [out] out 输出的H5Record
 * @return false 转换失败\n
 *         true 转换成功
 * @note 字符串格式为：Date,Open,Low,High,Close,Volume,Amount
 */
//-----------------------------------------------------------------------------
bool str_to_h5_record(const std::string& str, H5Record *out) {
    assert(out);
    std::vector<std::string> str_list;
    size_t pre = 0;
    size_t total = str.length();
    if (total == 0) {
        return false;
    }

    for (size_t i = 0; i < total; i++) {
        if (str.at(i) == ',') {
            if (i > pre) {
                str_list.push_back(str.substr(pre, i - pre));
            }
            pre = i + 1;
        }
    }

    if (pre < total - 1) {
        str_list.push_back(str.substr(pre, total - pre - 1));
    }

    if (str_list.size() != 7) {
        return false;
    }

    try {
        out->openPrice = 1000 * boost::lexical_cast<float>(str_list[1]);
        out->lowPrice =  1000 * boost::lexical_cast<float>(str_list[2]);
        out->highPrice =  1000 * boost::lexical_cast<float>(str_list[3]);
        out->closePrice =  1000 * boost::lexical_cast<float>(str_list[4]);
        //out->transCount =  boost::lexical_cast<unsigned long long>(str_list[5]);
        //out->transAmount =  boost::lexical_cast<unsigned long long>(str_list[6]);
        out->transCount =  boost::lexical_cast<double>(str_list[5]);
        out->transAmount =  boost::lexical_cast<double>(str_list[6]);

        size_t pos = str_list[0].find(' ');
        if (pos == std::string::npos) {
            return false;
        }

        using namespace boost::posix_time;
        ptime datetime(time_from_string(str_list[0]));
        bg::date day = datetime.date();
        time_duration td = datetime.time_of_day();
        out->datetime = (unsigned long long)day.year() * 100000000LL
                      + (unsigned long long)day.month() * 1000000LL
                      + (unsigned long long)day.day() * 10000LL
                      + (unsigned long long)td.hours() * 100LL
                      + (unsigned long long)td.minutes();

    } catch (boost::bad_lexical_cast& e) {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/**
 * 从csv文件中导入1分钟线数据到目标HDF5文件
 * @param file_name csv文件名
 * @param h5file 目标HDF5文件
 * @param table_name 目标表名
 */
//-----------------------------------------------------------------------------
void import_min1_data_from_csv(const std::string& file_name, const H5FilePtr& h5file,
                               const std::string& table_name) {
    std::ifstream file(file_name.c_str(), std::ifstream::in);
    if (!file)
        return;;

    H5::Group h5_group = h5_get_group(h5file, "/data");
    H5::DataSet h5_table = h5_get_data_table(h5_group, table_name);
    H5CompTypePtr h5_data_type = h5_get_data_type();
    hssize_t nrecords = h5_get_nrecords(h5_table);

    unsigned long long last_datetime = 0;
    H5Record last_record;
    if (nrecords > 0) {
        h5_read_records(h5_table, *h5_data_type, nrecords-1, 1, &last_record);
        last_datetime = last_record.datetime;
    }

    std::string line;
    while (std::getline(file, line)) {
        H5Record record;
        if (!str_to_h5_record(line, &record)) {
            //转换为H5Record信息失败，处理下一条
            continue;
        }

        //如果日期小于或等于最后日期，则跳过
        if (record.datetime <= last_datetime) {
            continue;
        }

        //如果日期无效，则跳过
        unsigned int cur_date = (unsigned int)(record.datetime / 10000LL);
        if (!invalid_date(cur_date)) {
            continue;
        }

        //最高价小于最低价，无效
        if (record.lowPrice > record.highPrice
                || record.openPrice > record.highPrice
                || record.closePrice > record.highPrice
                || record.openPrice < record.lowPrice
                || record.closePrice < record.lowPrice) {
            continue;
        }

        //价格或成交量数据中存在为0的数据，认为无效
        if (record.openPrice == 0 || record.highPrice == 0
                || record.lowPrice == 0 || record.closePrice == 0
                || record.transAmount == 0 || record.transCount == 0) {
            continue;
        }

        h5_append_records(h5_table, *h5_data_type, 1, &record);
        last_datetime = record.datetime;
    }

    file.close();
    h5_group.close();
}

//-----------------------------------------------------------------------------
/**
 * 导入1分钟线数据
 * @param db sqlite3数据库
 * @param h5 1分种线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void import_min1_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[import_min1_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[import_min1_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    if (!fs::is_directory(dir_path)) {
        std::cerr << "[import_min1_data] dir_path(" << dir_path << ") is not a directory!\n";
        return;
    }

    int count = 0;
    fs::directory_iterator end_iter;
    fs::directory_iterator dir_itr(dir_path);
    for (; dir_itr != end_iter; ++dir_itr) {
        if (fs::is_directory( dir_itr->status() ) ) {
            continue;
        }

        std::string code = fs::basename(dir_itr->path());
        if (code.length() <= 2) {
            continue;
        }

        code = code.substr(2, std::string::npos);
        /* 数据导入不判断，用于保存所有数据
        unsigned int stockid = get_stockid(db, marketid, code);
        if (stockid == 0) {
            //不在数据库中股票，不导入
            continue;
        }*/

        std::string table_name = market + code;
        import_min1_data_from_csv(dir_itr->path().string(), h5, table_name);
        count++;
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 根据HDF5文件的日K线数据，更新股票数据库中股票的有效性/起始日期/结束日期
 * @param db Sqlit3股票数据库
 * @param h5file 日K线对应的h5文件指针
 * @param table_name 日k线的数据表名（如SH000001)
 * @param stockid 需要更新的股票标识
 * @note 仅供update_all_stock_date函数调用
 */
//-----------------------------------------------------------------------------
void update_stock_date(const SqlitePtr& db, const H5FilePtr& h5file,
                       const std::string& table_name, unsigned int stockid) {
    char *zErrMsg=0;
    int rc;
    std::stringstream buf(std::stringstream::out);
    buf << "select valid, startDate, endDate from stock where stockid=" << stockid;
    StockInfo stock_info;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stock_info, &stock_info, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    unsigned int today = get_today_date();

    H5::Group h5_group = h5_get_group(h5file, "/data");
    //不能直接使用h5_get_data_table，会创建空数据集，造成脏数据
    //H5::DataSet h5_table = h5_get_data_table(h5_group, table_name);
    H5::DataSet h5_table;
    hssize_t nrecords = 0;
    if (h5_is_exist_data_table(h5_group, table_name)) {
        h5_table = h5_get_data_table(h5_group, table_name);
        nrecords = h5_get_nrecords(h5_table);
    }

    H5CompTypePtr h5_type = h5_get_data_type();
    //hssize_t nrecords = h5_get_nrecords(h5_table);
    if (nrecords == 0) {
        //如果没有历史K线数据，不管该股票是否有效，直接将其删除
        buf.str("");
        buf << "delete from stock where stockid=" << stockid;
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }

        buf.str("");
        buf << "delete from stkweight where stockid=" << stockid;
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }

        return;

        /*//如果没有K线数据，但该股票有效，则将其置为无效
        if (stock_info.valid == 1) {
            buf.str("");
            //buf << "update stock set valid=0, startDate=0, endDate=0 where stockid=" << stockid;
            buf << "update stock set valid=0,"
                << "startDate=" << today << ","
                << "endDate=" << today
                << " where stockid=" << stockid;
            rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                return;
            }
        }
        return;*/
    }

    H5Record h5_record;
    h5_read_records(h5_table, *h5_type, 0, 1, &h5_record);
    unsigned int start_date = (unsigned int)(h5_record.datetime / 10000);
    if (start_date < stock_info.startDate || stock_info.startDate == 0) {
        buf.str("");
        buf << "update stock set startDate=" << start_date << " where stockid=" << stockid;
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
    }

    if (stock_info.valid == 0) {
        h5_read_records(h5_table, *h5_type, nrecords-1, 1, &h5_record);
        unsigned int end_date = (unsigned int)(h5_record.datetime / 10000);
        if (end_date < stock_info.endDate) {
            buf.str("");
            buf << "update stock set endDate=" << end_date << " where stockid=" << stockid;
            rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                return;
            }
        }
    } else {
        if (stock_info.endDate != 99999999) {
            buf.str("");
            buf << "update stock set endDate=99999999 where stockid=" << stockid ;
            rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
            if( rc != SQLITE_OK ){
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                return;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/**
 * 更新指定市场的所有股票有效性/起始日期/结束日期信息，以及Market的最后日期
 * @param db sqlite3股票数据库
 * @param h5file 该市场对应的日K线HDF5文件指针
 * @param market 市场名称
 * @note 该函数应该在将所有日线数据导入HDF5文件后调用
 */
//-----------------------------------------------------------------------------
void update_all_stock_date(const SqlitePtr& db, const H5FilePtr& h5file,
                     const std::string& market) {
    assert(db);
    assert(h5file);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[update_all_stock_date] Invalid market" << market << "!\n";
        return;
    }

    char *zErrMsg = 0;
    int rc;
    std::list<StockCode> stock_list;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid,code from stock where marketid=" << marketid;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stockcode_list, &stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //启动SQL更新事务
    rc = sqlite3_exec(db.get(), "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //更新所有股票的有效日期信息
    std::list<StockCode>::iterator stock_iter = stock_list.begin();
    for (; stock_iter != stock_list.end(); ++stock_iter) {
        std::string table_name = market + stock_iter->code;
        update_stock_date(db, h5file, table_name, stock_iter->stockid);
    }

    //更新Market最后日期
    buf.str("");
    buf << "select code from market where marketid=" << marketid;
    std::string code;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_market_stock, &code, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    H5::Group h5_group = h5_get_group(h5file, "/data");
    H5::DataSet h5_table = h5_get_data_table(h5_group, market+code);
    H5CompTypePtr h5_type = h5_get_data_type();
    hssize_t nrecords = h5_get_nrecords(h5_table);
    if (nrecords != 0) {
        H5Record h5_record;
        h5_read_records(h5_table, *h5_type, nrecords-1, 1, &h5_record);
        buf.str("");
        buf << "update market set lastDate=" << h5_record.datetime / 10000LL << " where marketid=" << marketid;
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
        buf.str("");
        buf << "update LastDate set date=" << h5_record.datetime / 10000LL << " where id=1";
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
        }
    }

    //事务提交
    rc = sqlite3_exec(db.get(), "COMMIT", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

/*****************************************************************************
 *
 * 以下为通达信数据导入
 *
 *****************************************************************************/

//由tdx_import_stock_name调用
void tdx_import_stock_name_from_file(const SqlitePtr& db,
                                     const std::string& filename,
                                     const std::string& market) {
    std::map<std::string, std::string> new_stock_dict;

    std::ifstream file(filename.c_str(), std::ifstream::binary);
    if( !file ) {
        std::cerr << "[tdx_import_stock_name_from_file] Can't open file: "
                << filename << std::endl;
        return;
    }

    char buffer[315];
    char stockname[10];
    char stockcode[7];
    memset(buffer, 0 , 315);
    memset(stockname, 0, 10);
    memset(stockcode, 0, 7);

    file.read(buffer, 50);
    memset(buffer, 0 , 315);

    while (file.read(buffer, 314)) {
        memcpy(stockcode, buffer, 6);
        memcpy(stockname, buffer + 23, 8);
        memset(buffer, 0, 315);
        new_stock_dict[stockcode] = GBToUTF8(stockname);
        //std::cout << stockname  << " " << stockcode << std::endl;
    }

    file.close();

    unsigned int marketid = get_marketid(db, market);

    std::list<StockInfoByMarket> old_stock_list;
    char *zErrMsg=0;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid, code, name, valid from stock where marketid = " << marketid;
    int rc = sqlite3_exec(db.get(), buf.str().c_str(),
            callback_get_stock_info_by_market,
            &old_stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    //启动SQL更新事务
    rc = sqlite3_exec(db.get(), "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    std::map<std::string, unsigned int> old_stock_dict;

    std::map<std::string, std::string>::const_iterator new_dict_iter;
    std::list<StockInfoByMarket>::const_iterator old_iter = old_stock_list.begin();
    for (; old_iter != old_stock_list.end(); ++old_iter) {
        old_stock_dict[old_iter->code] = old_iter->stockid;

        new_dict_iter = new_stock_dict.find(old_iter->code);
        if (new_dict_iter == new_stock_dict.end()) {
            //新的代码表中无此股票，则置为无效
            if (old_iter->valid == 1) {
                buf.str("");
                buf << "update stock set valid=0 where stockid=" << old_iter->stockid;
                rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    std::cout << buf.str() << std::endl;
                    sqlite3_free(zErrMsg);
                    return;
                }
            }

        } else {
            //股票名称发生变化，更新股票名称;如果原无效，则置为有效
            if (old_iter->name != new_dict_iter->second) {
                buf.str("");
                buf << "update stock set name='" << new_dict_iter->second << "'"
                    << " where stockid=" << old_iter->stockid;
                rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    std::cout << buf.str() << std::endl;
                    sqlite3_free(zErrMsg);
                    return;
                }
            }

            if (old_iter->valid == 0) {
                buf.str("");
                buf << "update stock set valid=1, endDate=99999999 where stockid="
                    << old_iter->stockid;
                rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    std::cout << buf.str() << std::endl;
                    sqlite3_free(zErrMsg);
                    return;
                }
            }
        }
    }

    CodeTreePtr codeTree = create_CodeTree(db, market);
    if (!codeTree) {
        std::cerr << "[tdx_import_stock_name_from_file] Create CodeTree Failure!\n";
        return;
    }

    unsigned int today = get_today_date();

    for (new_dict_iter = new_stock_dict.begin();
            new_dict_iter != new_stock_dict.end(); ++new_dict_iter) {

        if (old_stock_dict.find(new_dict_iter->first) != old_stock_dict.end())
            continue;

        unsigned int stocktype = codeTree->getStockType(new_dict_iter->first);
        if (stocktype == 0)
            continue;

        buf.str("");
        buf << "insert into Stock(marketid, code, name, type, valid, startDate, endDate) "
            << "values ("
            << marketid << ","
            << "'" << new_dict_iter->first << "',"
            << "'" << new_dict_iter->second << "',"
            << stocktype << ","
            << "1,"
            << today << ","
            << "99999999)";
        rc = sqlite3_exec(db.get(), buf.str().c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            std::cout << buf.str() << std::endl;
            sqlite3_free(zErrMsg);
            return;
        }
    }

    //事务提交
    rc = sqlite3_exec(db.get(), "COMMIT", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

//-----------------------------------------------------------------------------
/**
 * 从通达信导入更新每只股票的名称和当前是否有效的标志
 * @detail 如果导入的代码表中不存在对应的代码，则认为该股已失效
 * @param db sqlite数据库
 * @param filename 大智慧的证券代码表文件名
 */
//-----------------------------------------------------------------------------
void tdx_import_stock_name(const SqlitePtr& db, const std::string& dirname) {
    assert(db);
    tdx_import_stock_name_from_file(db, dirname + "/T0002/hq_cache/shm.tnf", "SH");
    tdx_import_stock_name_from_file(db, dirname + "/T0002/hq_cache/szm.tnf", "SZ");
}


/*
 * 获取通达信日线文件记录数
 */
int tdx_get_day_data_count(const std::string& filename) {
    if (!fs::exists(filename))
        return 0;

    struct stat statbuf;
    stat(filename.c_str(), &statbuf);
    int size = statbuf.st_size;
    return size / sizeof(TdxDayData);
}

/*
 * 获取通达信日线文件指定记录的日期
 */
unsigned int tdx_get_day_data_date(std::ifstream& file, int pos) {
    file.seekg(pos * sizeof(TdxDayData), file.beg);
    unsigned int date = 0;
    file.read((char *)&date, sizeof(date));
    return date;
}


/*
 * 选择通达信日线文件中第一个大于等于lastdate的记录位置
 */
int tdx_day_data_find_pos(std::ifstream& file, unsigned int last_date) {
    file.seekg(0, file.end);
    int total = file.tellg() / sizeof(TdxDayData);

    int low = 0, high = total - 1;
    int mid = high / 2;
    while (mid <= high) {
        unsigned int cur_date = tdx_get_day_data_date(file, low);
        if (cur_date > last_date) {
            mid = low;
            break;
        }

        cur_date = tdx_get_day_data_date(file, high);
        if (cur_date <= last_date) {
            mid = high + 1;
            break;
        }

        cur_date = tdx_get_day_data_date(file, mid);
        if (cur_date <= last_date) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }

        mid = (low + high) / 2;
    }

    return mid;
}

//-----------------------------------------------------------------------------
/**
 * 从通达信文件中导入日线数据到目标HDF5文件
 * @param file_name 钱龙或大智慧数据文件名
 * @param h5file 目标HDF5文件
 * @param table_name 目标表名
 * @return 成功导入的数据量
 */
//-----------------------------------------------------------------------------
int tdx_import_day_data_from_file(const SqlitePtr& db,
                               const std::string& file_name,
                               const H5FilePtr& h5file,
                               const std::string& market,
                               const std::string& code) {
    int total = tdx_get_day_data_count(file_name);
    if (total == 0) {
        return 0;
    }

    std::ifstream file(file_name.c_str(), std::ifstream::binary);
    if (!file)
        return 0;

    unsigned int marketid = get_marketid(db, market);
    int stktype = get_stock_type(db, marketid, code);

    std::string table_name = market + code;
    H5CompTypePtr h5_data_type = h5_get_data_type();
    H5::Group h5_group = h5_get_group(h5file, "/data");

    unsigned int last_date = 0;
    H5::DataSet h5_table;
    bool is_exist_data_table = h5_is_exist_data_table(h5_group, table_name);
    if (is_exist_data_table) {
        h5_table = h5_get_data_table(h5_group, table_name);
        hssize_t nrecords = h5_get_nrecords(h5_table);
        H5Record last_record;
        if (nrecords > 0) {
            h5_read_records(h5_table, *h5_data_type, nrecords-1, 1, &last_record);
            last_date = (unsigned int)(last_record.datetime / 10000);
        }
    }

#if USE_FIND_POS
    int pos = tdx_day_data_find_pos(file, last_date);
    if (pos >= total) {
        file.close();
        h5_group.close();
        return 0;
    }

    file.seekg(pos * sizeof(TdxDayData), file.beg);
#endif

    std::vector<H5Record> h5_buffer;
    TdxDayData day_data;
    memset(&day_data, 0, sizeof(TdxDayData));
    while (file.read((char *)&day_data, sizeof(TdxDayData))){

#if !USE_FIND_POS
        //如果日期小于或等于最后日期，则跳过
        if (day_data.date <= last_date) {
            //std::cout << "invalid date(date <= last_date): " << last_record.datetime << " "
            //      << last_date << " " << day_data.date << " " << table_name << std::endl;
            continue;
        }
#endif

        //如果日期无效，则跳过
        if (!invalid_date(day_data.date)) {
            //std::cout << "!invalid_date(date): " << day_data.date << " " << table_name << std::endl;
            continue;
        }

        if (day_data.low > day_data.high
                || day_data.open > day_data.high
                || day_data.close > day_data.high
                || day_data.open < day_data.low
                || day_data.close < day_data.low
                || day_data.high < day_data.low) {
            continue;
        }

        if ((day_data.amount == 0 && day_data.count == 0)
                || day_data.open == 0 || day_data.high == 0
                || day_data.low == 0
                || day_data.close == 0) {
            continue;
        }

        H5Record h5_record;
        h5_record.datetime = (unsigned long long)day_data.date * 10000;
        h5_record.openPrice = day_data.open * 10;
        h5_record.highPrice = day_data.high * 10;
        h5_record.lowPrice = day_data.low * 10;
        h5_record.closePrice = day_data.close * 10;
        h5_record.transAmount = (unsigned long long)(day_data.amount * 0.001);
        if (stktype == 2) {
            h5_record.transCount = day_data.count;
        } else {
            h5_record.transCount = (unsigned long long)(day_data.count * 0.01);
        }

        h5_buffer.push_back(h5_record);
        //h5_append_records(h5_table, *h5_data_type, 1, &h5_record);

        memset(&day_data, 0 , sizeof(TdxDayData));
        last_date = day_data.date;
    }

    if (h5_buffer.size() > 0) {
        if (!is_exist_data_table)
            h5_table = h5_get_data_table(h5_group, table_name);

        h5_append_records(h5_table, *h5_data_type,
                          h5_buffer.size(), h5_buffer.data());
    }

    file.close();
    h5_group.close();
    return h5_buffer.size();
}


/*
 * 获取通达信分钟线文件记录数
 */
int tdx_get_min_data_count(const std::string& filename) {
    if (!fs::exists(filename))
        return 0;

    struct stat statbuf;
    stat(filename.c_str(), &statbuf);
    int size = statbuf.st_size;
    return size / sizeof(TdxMinData);
}

unsigned long long tdx_min_data_trans_date(unsigned short yymm, unsigned short hhmm) {
    int tmp_date = yymm >> 11;
    int remainder = yymm & 0x7ff;
    int year = tmp_date + 2004;
    int month = remainder / 100;
    int day = remainder % 100;
    int hh = hhmm / 60;
    int mm = hhmm % 60;
    return ((unsigned long long)year*100000000LL
          + (unsigned long long)month*1000000LL
          + (unsigned long long)day*10000LL
          + (unsigned long long)hh*100LL
          + (unsigned long long)mm);
}

/*
 * 获取通达信分钟线文件指定记录的日期
 */
unsigned long long tdx_get_min_data_date(std::ifstream& file, int pos) {
    file.seekg(pos * sizeof(TdxMinData), file.beg);
    unsigned short date[2];
    date[0] = 0, date[1] = 0;
    file.read((char *)&date, 4);
    return tdx_min_data_trans_date(date[0], date[1]);
}


/*
 * 选择通达信分钟线文件中第一个大于等于lastdate的记录位置
 */
int tdx_min_data_find_pos(std::ifstream& file, unsigned long long last_date) {
    file.seekg(0, file.end);
    int pos = file.tellg();
    int total = pos / sizeof(TdxMinData);

    int low = 0, high = total - 1;
    int mid = high / 2;
    while (mid <= high) {
        unsigned long long cur_date = tdx_get_min_data_date(file, low);
        if (cur_date > last_date) {
            mid = low;
            break;
        }

        cur_date = tdx_get_min_data_date(file, high);
        if (cur_date <= last_date) {
            mid = high + 1;
            break;
        }

        cur_date = tdx_get_min_data_date(file, mid);
        if (cur_date <= last_date) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }

        mid = (low + high) / 2;
    }

    return mid;
}

//-----------------------------------------------------------------------------
/**
 * 从通达信文件中导入1、5分钟线数据到目标HDF5文件
 * @param file_name 钱龙或大智慧数据文件名
 * @param h5file 目标HDF5文件
 * @param table_name 目标表名
 * @return 返回成功导入的数据量
 */
//-----------------------------------------------------------------------------
int tdx_import_min_data_from_file(const SqlitePtr& db,
                              const std::string& file_name,
                              const H5FilePtr& h5file,
                              const std::string& market,
                              const std::string& code) {
    int total = tdx_get_min_data_count(file_name);
    if (total == 0) {
        return 0;
    }

    std::ifstream file(file_name.c_str(), std::ifstream::binary);
    if (!file)
        return 0;

    unsigned int marketid = get_marketid(db, market);
    int stktype = get_stock_type(db, marketid, code);
    std::string table_name = market + code;

    H5CompTypePtr h5_data_type = h5_get_data_type();
    H5::Group h5_group = h5_get_group(h5file, "/data");

    unsigned long long last_datetime = 0;
    H5::DataSet h5_table;
    bool is_exist_data_table = h5_is_exist_data_table(h5_group, table_name);
    if (is_exist_data_table) {
        h5_table = h5_get_data_table(h5_group, table_name);
        hssize_t nrecords = h5_get_nrecords(h5_table);
        H5Record last_record;
        if (nrecords > 0) {
            h5_read_records(h5_table, *h5_data_type, nrecords-1, 1, &last_record);
            last_datetime = last_record.datetime;
        }
    }

#if USE_FIND_POS
    //int total = tdx_get_min_data_count(file);
    int pos = tdx_min_data_find_pos(file, last_datetime);
    if (pos >= total) {
        file.close();
        h5_group.close();
        return 0;
    }

    file.seekg(pos * sizeof(TdxMinData), file.beg);
#endif

    std::vector<H5Record> h5_buffer;
    TdxMinData data;
    unsigned int tempval = 0xFFFFFFFF;
    memset(&data, 0, sizeof(TdxMinData));
    while (file.read((char *)&data, sizeof(TdxMinData))){

        unsigned long long cur_datetime = tdx_min_data_trans_date(data.date, data.minute);

#if !USE_FIND_POS
        //如果日期小于或等于最后日期，则跳过
        if (cur_datetime <= last_datetime) {
            //std::cout << "invalid date(date <= last_date): " << day_data.date << " " << table_name << std::endl;
            continue;
        }
#endif

        //如果日期无效，则跳过
        unsigned int cur_date = (unsigned int)(cur_datetime / 10000LL);
        if (!invalid_date(cur_date)) {
            //std::cout << "!invalid_date(date): " << day_data.date << " " << table_name << std::endl;
            continue;
        }

        if (data.low > data.high
                || data.open > data.high
                || data.close > data.high
                || data.open < data.low
                || data.close < data.low
                || data.high < data.low) {
            continue;
        }

        if ((data.amount == 0 && data.count == 0)
                || data.open == 0 || data.high == 0
                || data.low == 0 || data.close == 0) {
            continue;
        }

        H5Record h5_record;
        h5_record.datetime    = cur_datetime;
        h5_record.openPrice   = (unsigned int)(data.open * 1000);
        h5_record.highPrice   = (unsigned int)(data.high * 1000);
        h5_record.lowPrice    = (unsigned int)(data.low * 1000);
        h5_record.closePrice  = (unsigned int)(data.close * 1000);
        h5_record.transAmount = (unsigned long long)(data.amount * 0.001);
        if (stktype == 2) {
            h5_record.transCount = data.count;
        } else {
            h5_record.transCount = (unsigned long long)(data.count * 0.01);
        }

        h5_buffer.push_back(h5_record);
        //h5_append_records(h5_table, *h5_data_type, 1, &h5_record);

        memset(&data, 0 , sizeof(TdxMinData));
        last_datetime = cur_datetime;
    }

    if (h5_buffer.size() > 0) {
        if (!is_exist_data_table)
            h5_table = h5_get_data_table(h5_group, table_name);

        h5_append_records(h5_table, *h5_data_type,
                          h5_buffer.size(), h5_buffer.data());
    }

    file.close();
    h5_group.close();
    return h5_buffer.size();
}

//-----------------------------------------------------------------------------
/**
 * 导入通达信日线数据
 * @param db sqlite3数据库
 * @param h5 日线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void tdx_import_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[tdx_import_day_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[tdx_import_day_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    char *zErrMsg = 0;
    int rc;
    std::list<StockCode> stock_list;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid,code from stock where marketid=" << marketid;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stockcode_list, &stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    int total = stock_list.size();
    int count = 0, cur = 0;
    std::list<StockCode>::const_iterator stk_iter = stock_list.begin();
    for (; stk_iter != stock_list.end(); ++stk_iter) {
        progress_bar(++cur, total);
        std::string tmp_market = market;
        boost::to_lower(tmp_market);
        std::string filename = dir_path.string()
                             + "/" + tmp_market + stk_iter->code + ".day";
        //std::cout << filename << std::endl;
        count += tdx_import_day_data_from_file(db, filename, h5, market, stk_iter->code);

        std::string table_name = market + stk_iter->code;
        update_h5_index(h5, table_name, INDEX_WEEK);
        update_h5_index(h5, table_name, INDEX_MONTH);
        update_h5_index(h5, table_name, INDEX_QUARTER);
        update_h5_index(h5, table_name, INDEX_HALFYEAR);
        update_h5_index(h5, table_name, INDEX_YEAR);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 导入通达信所有日线数据，不管数据库中是否存在该股票
 * @param db sqlite3数据库
 * @param h5 日线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void tdx_import_all_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[tdx_import_all_day_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[tdx_import_all_day_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    if (!fs::is_directory(dir_path)) {
        std::cerr << "[tdx_import_all_day_data] dir_path(" << dir_path << ") is not a directory!\n";
        return;
    }

    fs::directory_iterator end_iter;
    fs::directory_iterator count_itr(dir_path);
    int total = 0;
    for (; count_itr != end_iter; ++count_itr) {
        total++;
    }

    int count = 0, cur = 0;
    fs::directory_iterator dir_itr(dir_path);
    for (; dir_itr != end_iter; ++dir_itr) {
        progress_bar(++cur, total);
        if (fs::is_directory( dir_itr->status() ) ) {
            continue;
        }

        std::string table_name = fs::basename(dir_itr->path());
        if (table_name.length() != 8) {
            continue;
        }

        boost::to_upper(table_name);
        if (table_name.substr(0, 2) != market) {
            continue;
        }

        std::string code = table_name.substr(2);

        count += tdx_import_day_data_from_file(db, dir_itr->path().string(),
                                               h5, market, code);

        update_h5_index(h5, table_name, INDEX_WEEK);
        update_h5_index(h5, table_name, INDEX_MONTH);
        update_h5_index(h5, table_name, INDEX_QUARTER);
        update_h5_index(h5, table_name, INDEX_HALFYEAR);
        update_h5_index(h5, table_name, INDEX_YEAR);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 导入通达信1、5分钟线数据
 * @param db sqlite3数据库
 * @param h5 5分种线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void tdx_import_min_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[tdx_import_min_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[tdx_import_min_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    bool is_5min = true;
    std::string suffix;
    if (dir_path.string().find("fzline") != std::string::npos) {
        suffix = ".lc5";
    } else {
        suffix = ".lc1";
        is_5min = false;
    }

    char *zErrMsg = 0;
    int rc;
    std::list<StockCode> stock_list;
    std::stringstream buf(std::stringstream::out);
    buf << "select stockid,code from stock where marketid=" << marketid;
    rc = sqlite3_exec(db.get(), buf.str().c_str(), callback_get_stockcode_list, &stock_list, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return;
    }

    int total = stock_list.size();
    int count = 0, cur = 0;
    std::list<StockCode>::const_iterator stk_iter = stock_list.begin();
    for (; stk_iter != stock_list.end(); ++stk_iter) {
        progress_bar(++cur, total);
        std::string tmp_market = market;
        boost::to_lower(tmp_market);
        std::string filename = dir_path.string()
                             + "/" + tmp_market + stk_iter->code + suffix;

        std::string table_name = market + stk_iter->code;
        count += tdx_import_min_data_from_file(db, filename, h5, market, stk_iter->code);

        if (is_5min) {
            update_h5_index(h5, table_name, INDEX_MIN15);
            update_h5_index(h5, table_name, INDEX_MIN30);
            update_h5_index(h5, table_name, INDEX_MIN60);
        }

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

//-----------------------------------------------------------------------------
/**
 * 导入通达信全部1、5分钟线数据，不过该股票是否在数据库中存在
 * @param db sqlite3数据库
 * @param h5 5分种线数据存放目的地HDF5文件指针
 * @param market 市场标识
 * @param dir_path 源数据所在目录
 */
//-----------------------------------------------------------------------------
void tdx_import_all_min_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path) {
    assert(db);
    assert(h5);
    unsigned int marketid = get_marketid(db, market);
    if (marketid == 0) {
        std::cerr << "[tdx_import_min_data] Invalid market" << market << "!\n";
        return;
    }

    if (!fs::exists(dir_path)){
        std::cerr << "[tdx_import_min_data] dir_path(" << dir_path << ") not exist!\n";
        return;
    }

    if (!fs::is_directory(dir_path)) {
        std::cerr << "[tdx_import_min_data] dir_path(" << dir_path << ") is not a directory!\n";
        return;
    }

    fs::directory_iterator end_iter;
    fs::directory_iterator count_itr(dir_path);
    int total = 0;
    for (; count_itr != end_iter; ++count_itr) {
        total++;
    }

    int count = 0, cur = 0;
    fs::directory_iterator dir_itr(dir_path);
    for (; dir_itr != end_iter; ++dir_itr) {
        progress_bar(++cur, total);
        if (fs::is_directory( dir_itr->status() ) ) {
            continue;
        }

        std::string table_name = fs::basename(dir_itr->path());
        boost::to_upper(table_name);
        if (table_name.length() != 8) {
            continue;
        }

        if (table_name.substr(0,2) != market) {
            continue;
        }

        std::string code = table_name.substr(2);

        count += tdx_import_min_data_from_file(db, dir_itr->path().string(),
                                               h5, market, code);

        update_h5_index(h5, table_name, INDEX_MIN15);
        update_h5_index(h5, table_name, INDEX_MIN30);
        update_h5_index(h5, table_name, INDEX_MIN60);

        h5->flush(H5F_SCOPE_LOCAL);
    }

    std::cout << "导入数量：" << count << std::endl;
}

