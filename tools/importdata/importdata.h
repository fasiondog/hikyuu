/*
 * importdata.h
 *
 *  Created on: 2010-10-26
 *      Author: fasiondog
 */

#ifndef IMPORTDATA_H_
#define IMPORTDATA_H_

#include <stdlib.h>
#include <sqlite3.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include "CodeTree.h"

#if defined(BOOST_MSVC)
#include <cpp/H5Cpp.h>
#else
#include <hdf5/serial/H5Cpp.h>
#endif

#if !(defined(WIN32) && defined(_WIN32))
#include <sys/time.h>
#endif

#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#define access(f,m) _access((f),(m))
#endif

namespace fs = boost::filesystem;

typedef boost::shared_ptr<sqlite3> SqlitePtr;

#if !(defined(WIN32) && defined(_WIN32))
typedef struct timeval MY_TIME_VALUE;
//double mydifftime(struct timeval& end, struct timeval& start);
#else
typedef DWORD MY_TIME_VALUE;
void gettimeofday(MY_TIME_VALUE *, MY_TIME_VALUE *);
#endif

double mydifftime(MY_TIME_VALUE& end, MY_TIME_VALUE& start);

//打开指定的sqlite3数据库文件
SqlitePtr open_db(const std::string& dbname);

//根据指定的数据库创建脚本，在数据库中创建相关表格、索引及基础数据
bool create_database(const SqlitePtr& db, const std::string& filename);

void import_stock_weight(const SqlitePtr& db,
                         const std::string& market,
                         const std::string& path);

void dzh_import_stock_name(const SqlitePtr& db, const std::string& filename);
void tdx_import_stock_name(const SqlitePtr& db, const std::string& dirname);

std::string GBToUTF8(char *src_str);
bool invalid_date(unsigned int date);

//构建指定市场的证券代码规则树
CodeTreePtr create_CodeTree(const SqlitePtr& db, const std::string& market);

unsigned int get_marketid(const SqlitePtr& db, const std::string& market);
unsigned int get_stockid(const SqlitePtr& db, unsigned int marketid, const std::string& code);

/******************************************************************************
*
* 以下为HDF5文件操作
*
******************************************************************************/
typedef boost::shared_ptr<H5::H5File> H5FilePtr;
typedef boost::shared_ptr<H5::CompType> H5CompTypePtr;

//该数据结构仅供读取h5file时使用
class H5Record {
public: 
    unsigned long long datetime;
    unsigned int openPrice;
    unsigned int highPrice;
    unsigned int lowPrice;
    unsigned int closePrice;
    unsigned long long transAmount; //成交金额
    unsigned long long transCount;  //成交量
};

//该数据结构仅供读取h5file时使用
struct H5IndexRecord{
    unsigned long long datetime;
    unsigned long long start;
};

H5FilePtr h5_open_file(const std::string& filename);
void dzh_import_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);
void dzh_import_all_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);
void dzh_import_min5_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);
void dzh_import_all_min5_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);

void import_min1_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);

void tdx_import_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);
void tdx_import_min_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);
void tdx_import_all_day_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);
void tdx_import_all_min_data(const SqlitePtr& db, const H5FilePtr& h5,
                     const std::string& market, const fs::path& dir_path);

void update_all_stock_date(const SqlitePtr& db, const H5FilePtr& h5file,
                     const std::string& market);

/******************************************************************************
 * 钱龙与大智慧的K线数据结构
 *****************************************************************************/
struct QianLongData {
    unsigned int date;
    unsigned int open;
    unsigned int high;
    unsigned int low;
    unsigned int close;
    unsigned int amount;
    unsigned int count;
    char other[12];
};

bool invalid_QianLongData(const QianLongData& data);

/******************************************************************************
 * 通达信K线数据结构
 *****************************************************************************/
struct TdxDayData {
    unsigned int date;
    unsigned int open;
    unsigned int high;
    unsigned int low;
    unsigned int close;
    float        amount;
    unsigned int count;
    unsigned int other;
};

struct TdxMinData {
    unsigned short date;
    unsigned short minute;
    float open;
    float high;
    float low;
    float close;
    float amount;
    unsigned int count;
    unsigned int other;
};

struct TdxStockCodeTable {
    char code[6];
    char unknown[17];
    char name[8];
    char other[283];
};

enum H5_INDEX_TYPE {
    INDEX_WEEK,
    INDEX_MONTH,
    INDEX_QUARTER,
    INDEX_HALFYEAR,
    INDEX_YEAR,
    INDEX_MIN15,
    INDEX_MIN30,
    INDEX_MIN60
};

#endif /* IMPORTDATA_H_ */
