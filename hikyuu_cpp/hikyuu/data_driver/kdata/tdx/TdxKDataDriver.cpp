/*
 * TdxKDataDriver.cpp
 *
 *  Created on: 2017年10月12日
 *      Author: fasiondog
 */

#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include "TdxKDataDriver.h"

namespace hku {

struct TdxDayData {
    hku_uint32 date;
    hku_uint32 open;
    hku_uint32 high;
    hku_uint32 low;
    hku_uint32 close;
    float     amount;
    hku_uint32 vol;
    hku_uint32 other;

    Datetime getDatetime() {
        return Datetime(hku_uint64(date) * 10000);
    }

    void toKRecord(KRecord& record) {
        record.datetime    = Datetime(hku_uint64(date) * 10000);
        record.openPrice   = price_t(open) * 0.01;
        record.highPrice   = price_t(high) * 0.01;
        record.lowPrice    = price_t(low) * 0.01;
        record.closePrice  = price_t(close) * 0.01;
        record.transAmount = price_t(amount) * 0.0001;
        record.transCount  = price_t(vol);
        //注：指数的成交量单位是股，和HDF5数据不同，HDF是手
    }
};

struct TdxMinData {
    unsigned short date;
    unsigned short minute;
    float open;
    float high;
    float low;
    float close;
    float amount;
    hku_uint32 vol;
    hku_uint32 other;

    Datetime getDatetime() {
        int tmp_date = date >> 11;
        int remainder = date & 0x7ff;
        int year = tmp_date + 2004;
        int month = remainder / 100;
        int day = remainder % 100;
        int hh = minute / 60;
        int mm = minute % 60;
        return Datetime(year, month, day, hh, mm);
    }

    void toKRecord(KRecord& record) {
        record.datetime    = getDatetime();
        record.openPrice   = price_t(open);
        record.highPrice   = price_t(high);
        record.lowPrice    = price_t(low);
        record.closePrice  = price_t(close);
        record.transAmount = price_t(amount);
        record.transCount  = price_t(vol);
    }
};

TdxKDataDriver::TdxKDataDriver(): KDataDriver("tdx") {

}

TdxKDataDriver::~TdxKDataDriver() {

}

bool TdxKDataDriver::_init() {
    try {
        m_dirname = getParam<string>("dir");

    } catch(...) {
        return false;
    }

    return true;
}

void TdxKDataDriver::
loadKData(const string& market, const string& code,
        KQuery::KType ktype, size_t start_ix, size_t end_ix,
        KRecordListPtr out_buffer) {
    switch (ktype) {
    case KQuery::MIN:
    case KQuery::MIN5:
        _loadMinKData(market, code, ktype, start_ix, end_ix, out_buffer);
        break;

    case KQuery::MIN15:
    case KQuery::MIN30:
    case KQuery::MIN60:
        break;

    case KQuery::DAY:
        _loadDayKData(market, code, ktype, start_ix, end_ix, out_buffer);
        break;

    case KQuery::WEEK:
    case KQuery::MONTH:
    case KQuery::QUARTER:
    case KQuery::HALFYEAR:
    case KQuery::YEAR:
        break;

    default:
        break;
    }
}


void TdxKDataDriver::
_loadDayKData(const string& market, const string& code,
        KQuery::KType ktype, size_t start_ix, size_t end_ix,
        KRecordListPtr out_buffer) {
    assert(KQuery::DAY == ktype);

    size_t total = getCount(market, code ,ktype);
    if (0 == total || start_ix >= total) {
        return;
    }

    string filename = _getFileName(market, code, ktype);
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return;
    }

    struct TdxDayData tdx_data;
    file.seekg(start_ix * sizeof(tdx_data));

    size_t i = start_ix;
    size_t stop = total < end_ix ? total : end_ix;
    while (!file.eof() && i < stop) {
        KRecord record;
        file.read((char *)&tdx_data, sizeof(tdx_data));
        tdx_data.toKRecord(record);
        out_buffer->push_back(record);
        i++;
    }

    file.close();
    return;
}

void TdxKDataDriver::
_loadMinKData(const string& market, const string& code,
        KQuery::KType ktype, size_t start_ix, size_t end_ix,
        KRecordListPtr out_buffer) {
    assert(KQuery::MIN == ktype || KQuery::MIN5 == ktype);

    size_t total = getCount(market, code ,ktype);
    if (0 == total || start_ix >= total) {
        return;
    }

    string filename = _getFileName(market, code, ktype);
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return;
    }

    struct TdxMinData tdx_data;
    file.seekg(start_ix * sizeof(tdx_data));

    size_t i = start_ix;
    size_t stop = total < end_ix ? total : end_ix;
    while (!file.eof() && i < stop) {
        KRecord record;
        file.read((char *)&tdx_data, sizeof(tdx_data));
        tdx_data.toKRecord(record);
        out_buffer->push_back(record);
        i++;
    }

    file.close();
    return;
}


KRecord TdxKDataDriver::
getKRecord(const string& market, const string& code,
          size_t pos, KQuery::KType ktype) {
    KRecord record;
    switch (ktype) {
    case KQuery::MIN:
    case KQuery::MIN5:
        record = _getMinKRecord(market, code, pos, ktype);
        break;

    case KQuery::MIN15:
    case KQuery::MIN30:
    case KQuery::MIN60:
        break;

    case KQuery::DAY:
        record = _getDayKRecord(market, code, pos, ktype);
        break;

    case KQuery::WEEK:
    case KQuery::MONTH:
    case KQuery::QUARTER:
    case KQuery::HALFYEAR:
    case KQuery::YEAR:
    default:
        break;
    }

    return record;
}


KRecord TdxKDataDriver::
_getDayKRecord(const string& market, const string& code,
        size_t pos, KQuery::KType ktype) {
    assert(KQuery::DAY == ktype);

    KRecord record;
    size_t total = getCount(market, code, ktype);
    if (0 == total || pos >= total) {
        return record;
    }

    string filename = _getFileName(market, code, ktype);
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return record;
    }

    file.seekg(pos * sizeof(TdxDayData));
    struct TdxDayData tdx_data;
    file.read((char *)&tdx_data, sizeof(tdx_data));
    tdx_data.toKRecord(record);
    file.close();
    return record;
}


KRecord TdxKDataDriver::
_getMinKRecord(const string& market, const string& code,
        size_t pos, KQuery::KType ktype) {
    assert(KQuery::MIN == ktype || KQuery::MIN5 == ktype);

    KRecord record;
    size_t total = getCount(market, code, ktype);
    if (0 == total || pos >= total) {
        return record;
    }

    string filename = _getFileName(market, code, ktype);
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return record;
    }

    file.seekg(pos * sizeof(TdxMinData));
    struct TdxMinData tdx_data;
    file.read((char *)&tdx_data, sizeof(tdx_data));
    tdx_data.toKRecord(record);

    file.close();
    return record;
}


bool TdxKDataDriver::
getIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    assert(KQuery::DATE == query.queryType());

    if (KQuery::DAY == query.kType()) {
        return _getDayIndexRangeByDate(market, code, query, out_start, out_end);

    } else if (KQuery::MIN == query.kType() || KQuery::MIN5 == query.kType()) {
        return _getMinIndexRangeByDate(market, code ,query, out_start, out_end);
    }

    else {
        //return _getOtherIndexRangeByDate(market, code, query,
        //        out_start, out_end);
    }

    return false;
}

bool TdxKDataDriver::
_getDayIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    out_start = 0;
    out_end = 0;

    if (query.kType() != KQuery::DAY) {
        return false;
    }

    if(query.startDatetime() >= query.endDatetime()
    || query.startDatetime() > Datetime::max()) {
        return false;
    }

    string filename = _getFileName(market, code, query.kType());
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return false;
    }

    size_t total = getCount(market, code, query.kType());
    if (0 == total) {
        return false;
    }

    Datetime start_number = query.startDatetime();
    Datetime end_number = query.endDatetime();

    struct TdxDayData tdx_data;

    size_t len = sizeof(tdx_data);
    size_t mid, low = 0, high = total - 1;
    size_t startpos = 0, endpos = 0;
    while(low <= high) {
        file.seekg(high * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if(start_number > tdx_data.getDatetime()) {
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if(tdx_data.getDatetime() >= start_number) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        file.seekg(mid * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if(start_number > tdx_data.getDatetime()) {
            low = mid + 1;
        }else {
            high = mid - 1;
        }
    }

    if(mid >= total) {
        file.close();
        return false;
    }

    startpos = mid;

    low = mid;
    high = total - 1;
    while(low <= high){
        file.seekg(high * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if( end_number > tdx_data.getDatetime() ){
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if( tdx_data.getDatetime() >= end_number ){
            mid = low;
            break;
        }

        mid = (low + high)/2;
        file.seekg(mid * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if( end_number > tdx_data.getDatetime()){
            low = mid + 1;
        }else{
            high = mid - 1;
        }
    }

    endpos = mid >= total ? total : mid;
    if(startpos >= endpos) {
        file.close();
        return false;
    }

    file.close();

    out_start = startpos;
    out_end = endpos;
    return true;
}


bool TdxKDataDriver::
_getMinIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    out_start = 0;
    out_end = 0;

    if (query.kType() != KQuery::MIN && query.kType() != KQuery::MIN5) {
        return false;
    }

    if(query.startDatetime() >= query.endDatetime()
    || query.startDatetime() > Datetime::max()) {
        return false;
    }

    string filename = _getFileName(market, code, query.kType());
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return false;
    }

    size_t total = getCount(market, code, query.kType());
    if (0 == total) {
        return false;
    }

    Datetime start_number = query.startDatetime();
    Datetime end_number = query.endDatetime();

    struct TdxMinData tdx_data;

    size_t len = sizeof(tdx_data);
    size_t mid, low = 0, high = total - 1;
    size_t startpos = 0, endpos = 0;
    while(low <= high) {
        file.seekg(high * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if(start_number > tdx_data.getDatetime()) {
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if(tdx_data.getDatetime() >= start_number) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        file.seekg(mid * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if(start_number > tdx_data.getDatetime()) {
            low = mid + 1;
        }else {
            high = mid - 1;
        }
    }

    if(mid >= total) {
        file.close();
        return false;
    }

    startpos = mid;

    low = mid;
    high = total - 1;
    while(low <= high){
        file.seekg(high * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if( end_number > tdx_data.getDatetime() ){
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if( tdx_data.getDatetime() >= end_number ){
            mid = low;
            break;
        }

        mid = (low + high)/2;
        file.seekg(mid * len, file.beg);
        file.read((char *)&tdx_data, sizeof(tdx_data));
        if( end_number > tdx_data.getDatetime()){
            low = mid + 1;
        }else{
            high = mid - 1;
        }
    }

    endpos = mid >= total ? total : mid;
    if(startpos >= endpos) {
        file.close();
        return false;
    }

    file.close();

    out_start = startpos;
    out_end = endpos;
    return true;
}


string TdxKDataDriver::
_getFileName(const string& market, const string& code, KQuery::KType ktype) {
    string filename;
    switch (ktype) {
    case KQuery::MIN:
        filename = m_dirname + "\\" + market + "\\minline\\" + market + code + ".lc1";
        break;

    case KQuery::MIN5:
    case KQuery::MIN15:
    case KQuery::MIN30:
    case KQuery::MIN60:
        filename = m_dirname + "\\" + market + "\\fzline\\" + market + code + ".lc5";
        break;

    case KQuery::DAY:
    case KQuery::WEEK:
    case KQuery::MONTH:
    case KQuery::QUARTER:
    case KQuery::HALFYEAR:
    case KQuery::YEAR:
        filename = m_dirname + "\\" + market + "\\lday\\" + market + code + ".day";
        break;

    default:
        break;
    }

    return filename;
}

size_t TdxKDataDriver::
getCount(const string& market, const string& code, KQuery::KType ktype) {
    size_t count = 0;
    if (KQuery::DAY == ktype || KQuery::MIN5 == ktype || KQuery::MIN == ktype) {
        count =  _getBaseCount(market, code, ktype);
    }

    return count;
}


size_t TdxKDataDriver::
_getBaseCount(const string& market, const string& code, KQuery::KType ktype) {
    assert(KQuery::DAY == ktype || KQuery::MIN5 == ktype || KQuery::MIN == ktype);
    string filename = _getFileName(market, code ,ktype);
    if (filename.empty()) {
        return 0;
    }

    size_t count = 0;
    struct stat info;
    if (0 == stat(filename.c_str(), &info)) {
        count = info.st_size / sizeof(TdxDayData);
    }

    return count;
}

} /* namespace hku */
