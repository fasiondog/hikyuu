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
    uint32_t date;
    uint32_t open;
    uint32_t high;
    uint32_t low;
    uint32_t close;
    float amount;
    uint32_t vol;
    uint32_t other;

    Datetime getDatetime() {
        return Datetime(uint64_t(date) * 10000);
    }

    void toKRecord(KRecord& record) {
        record.datetime = Datetime(uint64_t(date) * 10000);
        record.openPrice = price_t(open) * 0.01;
        record.highPrice = price_t(high) * 0.01;
        record.lowPrice = price_t(low) * 0.01;
        record.closePrice = price_t(close) * 0.01;
        record.transAmount = price_t(amount) * 0.0001;
        record.transCount = price_t(vol);
        // 注：指数的成交量单位是股，和HDF5数据不同，HDF是手
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
    uint32_t vol;
    uint32_t other;

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
        record.datetime = getDatetime();
        record.openPrice = price_t(open);
        record.highPrice = price_t(high);
        record.lowPrice = price_t(low);
        record.closePrice = price_t(close);
        record.transAmount = price_t(amount);
        record.transCount = price_t(vol);
    }
};

TdxKDataDriver::TdxKDataDriver() : KDataDriver("tdx") {}

TdxKDataDriver::~TdxKDataDriver() {}

bool TdxKDataDriver::_init() {
    try {
        m_dirname = getParam<string>("dir");

    } catch (...) {
        return false;
    }

    return true;
}

KRecordList TdxKDataDriver::getKRecordList(const string& market, const string& code,
                                           const KQuery& query) {
    KRecordList result;
    auto ktype = query.kType();
    HKU_WARN_IF_RETURN(!(ktype == KQuery::MIN || ktype == KQuery::MIN5 || ktype == KQuery::DAY),
                       result, "The driver({}) don't support the ktype: {}", name(), ktype);

    if (query.queryType() == KQuery::INDEX) {
        if (ktype == KQuery::DAY) {
            result = _getDayKRecordList(market, code, ktype, query.start(), query.end());
        } else {
            result = _getMinKRecordList(market, code, ktype, query.start(), query.end());
        }
    } else {
        HKU_INFO("Query by date are not supported!");
    }

    return result;
}

KRecordList TdxKDataDriver::_getDayKRecordList(const string& market, const string& code,
                                               const KQuery::KType& ktype, size_t start_ix,
                                               size_t end_ix) {
    KRecordList result;
    size_t total = getCount(market, code, ktype);
    HKU_IF_RETURN(0 == total || start_ix >= total, result);

    string filename = _getFileName(market, code, ktype);
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    HKU_IF_RETURN(!file, result);

    struct TdxDayData tdx_data;
    file.seekg(start_ix * sizeof(tdx_data));

    size_t i = start_ix;
    size_t stop = total < end_ix ? total : end_ix;
    while (!file.eof() && i < stop) {
        KRecord record;
        file.read((char*)&tdx_data, sizeof(tdx_data));
        tdx_data.toKRecord(record);
        result.push_back(record);
        i++;
    }

    file.close();
    return result;
}

KRecordList TdxKDataDriver::_getMinKRecordList(const string& market, const string& code,
                                               const KQuery::KType& ktype, size_t start_ix,
                                               size_t end_ix) {
    assert(KQuery::MIN == ktype || KQuery::MIN5 == ktype);
    KRecordList result;

    size_t total = getCount(market, code, ktype);
    HKU_IF_RETURN(0 == total || start_ix >= total, result);

    string filename = _getFileName(market, code, ktype);
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file) {
        return result;
    }

    struct TdxMinData tdx_data;
    file.seekg(start_ix * sizeof(tdx_data));

    size_t i = start_ix;
    size_t stop = total < end_ix ? total : end_ix;
    while (!file.eof() && i < stop) {
        KRecord record;
        file.read((char*)&tdx_data, sizeof(tdx_data));
        tdx_data.toKRecord(record);
        result.push_back(record);
        i++;
    }

    file.close();
    return result;
}

bool TdxKDataDriver::getIndexRangeByDate(const string& market, const string& code,
                                         const KQuery& query, size_t& out_start, size_t& out_end) {
    assert(KQuery::DATE == query.queryType());

    if (KQuery::DAY == query.kType()) {
        return _getDayIndexRangeByDate(market, code, query, out_start, out_end);

    } else if (KQuery::MIN == query.kType() || KQuery::MIN5 == query.kType()) {
        return _getMinIndexRangeByDate(market, code, query, out_start, out_end);
    }

    else {
        // return _getOtherIndexRangeByDate(market, code, query,
        //        out_start, out_end);
    }

    return false;
}

bool TdxKDataDriver::_getDayIndexRangeByDate(const string& market, const string& code,
                                             const KQuery& query, size_t& out_start,
                                             size_t& out_end) {
    out_start = 0;
    out_end = 0;

    HKU_IF_RETURN(query.kType() != KQuery::DAY, false);
    HKU_IF_RETURN(
      query.startDatetime() >= query.endDatetime() || query.startDatetime() > Datetime::max(),
      false);

    string filename = _getFileName(market, code, query.kType());
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    HKU_IF_RETURN(!file, false);

    size_t total = getCount(market, code, query.kType());
    HKU_IF_RETURN(0 == total, false);

    Datetime start_number = query.startDatetime();
    Datetime end_number = query.endDatetime();

    struct TdxDayData tdx_data;

    size_t len = sizeof(tdx_data);
    size_t mid = total, low = 0, high = total - 1;
    size_t startpos = 0, endpos = 0;
    while (low <= high) {
        file.seekg(high * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (start_number > tdx_data.getDatetime()) {
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (tdx_data.getDatetime() >= start_number) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        file.seekg(mid * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (start_number > tdx_data.getDatetime()) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (mid >= total) {
        file.close();
        return false;
    }

    startpos = mid;

    low = mid;
    high = total - 1;
    while (low <= high) {
        file.seekg(high * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (end_number > tdx_data.getDatetime()) {
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (tdx_data.getDatetime() >= end_number) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        file.seekg(mid * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (end_number > tdx_data.getDatetime()) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    endpos = mid >= total ? total : mid;
    if (startpos >= endpos) {
        file.close();
        return false;
    }

    file.close();

    out_start = startpos;
    out_end = endpos;
    return true;
}

bool TdxKDataDriver::_getMinIndexRangeByDate(const string& market, const string& code,
                                             const KQuery& query, size_t& out_start,
                                             size_t& out_end) {
    out_start = 0;
    out_end = 0;

    HKU_IF_RETURN(query.kType() != KQuery::MIN && query.kType() != KQuery::MIN5, false);
    HKU_IF_RETURN(
      query.startDatetime() >= query.endDatetime() || query.startDatetime() > Datetime::max(),
      false);

    string filename = _getFileName(market, code, query.kType());
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    HKU_IF_RETURN(!file, false);

    size_t total = getCount(market, code, query.kType());
    HKU_IF_RETURN(0 == total, false);

    Datetime start_number = query.startDatetime();
    Datetime end_number = query.endDatetime();

    struct TdxMinData tdx_data;

    size_t len = sizeof(tdx_data);
    size_t mid = total, low = 0, high = total - 1;
    size_t startpos = 0, endpos = 0;
    while (low <= high) {
        file.seekg(high * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (start_number > tdx_data.getDatetime()) {
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (tdx_data.getDatetime() >= start_number) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        file.seekg(mid * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (start_number > tdx_data.getDatetime()) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (mid >= total) {
        file.close();
        return false;
    }

    startpos = mid;

    low = mid;
    high = total - 1;
    while (low <= high) {
        file.seekg(high * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (end_number > tdx_data.getDatetime()) {
            mid = high + 1;
            break;
        }

        file.seekg(low * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (tdx_data.getDatetime() >= end_number) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        file.seekg(mid * len, file.beg);
        file.read((char*)&tdx_data, sizeof(tdx_data));
        if (end_number > tdx_data.getDatetime()) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    endpos = mid >= total ? total : mid;
    if (startpos >= endpos) {
        file.close();
        return false;
    }

    file.close();

    out_start = startpos;
    out_end = endpos;
    return true;
}

string TdxKDataDriver::_getFileName(const string& market, const string& code,
                                    const KQuery::KType& ktype) {
    string filename;
    if (ktype == KQuery::MIN) {
        filename = m_dirname + "\\" + market + "\\minline\\" + market + code + ".lc1";
    } else if (ktype == KQuery::MIN5 || ktype == KQuery::MIN15 || ktype == KQuery::MIN30 ||
               ktype == KQuery::MIN60 || ktype == KQuery::HOUR2) {
        filename = m_dirname + "\\" + market + "\\fzline\\" + market + code + ".lc5";
    } else if (ktype == KQuery::DAY || ktype == KQuery::WEEK || ktype == KQuery::MONTH ||
               ktype == KQuery::QUARTER || ktype == KQuery::HALFYEAR || ktype == KQuery::YEAR) {
        filename = m_dirname + "\\" + market + "\\lday\\" + market + code + ".day";
    } else {
        HKU_WARN("Don't support this ktype: {}", ktype);
    }

    return filename;
}

size_t TdxKDataDriver::getCount(const string& market, const string& code,
                                const KQuery::KType& ktype) {
    size_t count = 0;
    if (KQuery::DAY == ktype || KQuery::MIN5 == ktype || KQuery::MIN == ktype) {
        count = _getBaseCount(market, code, ktype);
    }

    return count;
}

size_t TdxKDataDriver::_getBaseCount(const string& market, const string& code,
                                     const KQuery::KType& ktype) {
    assert(KQuery::DAY == ktype || KQuery::MIN5 == ktype || KQuery::MIN == ktype);
    string filename = _getFileName(market, code, ktype);
    HKU_IF_RETURN(filename.empty(), 0);

    size_t count = 0;
    struct stat info;
    if (0 == stat(filename.c_str(), &info)) {
        count = info.st_size / sizeof(TdxDayData);
    }

    return count;
}

} /* namespace hku */
