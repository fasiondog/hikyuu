/*
 * KDataTempCsvDriver.cpp
 *
 *  Created on: 2017年7月30日
 *      Author: fasiondog
 */

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "hikyuu/utilities/Log.h"
#include "KDataTempCsvDriver.h"

namespace hku {

KDataTempCsvDriver::~KDataTempCsvDriver() {}

KDataTempCsvDriver::KDataTempCsvDriver() : KDataTempCsvDriver("", "") {}

KDataTempCsvDriver::KDataTempCsvDriver(const string& day_filename, const string& min_filename)
: KDataDriver("TMPCSV"), m_day_filename(day_filename), m_min_filename(min_filename) {
    for (int i = 0; i < LAST; ++i) {
        m_column[i] = Null<size_t>();
    }

    m_token_buf.reserve(LAST);
}

void KDataTempCsvDriver::_get_token(const string& line) {
    m_token_buf.clear();

    string token;
    size_t pos = 0, prepos = 0;
    pos = line.find(',', prepos);
    while (pos != line.npos) {
        token.assign(line, prepos, pos - prepos);
        boost::trim(token);
        m_token_buf.push_back(token);
        prepos = pos + 1;
        pos = line.find(',', prepos);
    }

    if (prepos != pos) {
        token.assign(line, prepos, pos);
        boost::trim(token);
        m_token_buf.push_back(token);
    }
}

void KDataTempCsvDriver::_get_title_column(const string& line) {
    _get_token(line);

    int total = (int)m_token_buf.size();
    for (int i = 0; i < total; ++i) {
        string token = m_token_buf[i];
        to_upper(token);

        if ("DATE" == token || "DATETIME" == token || "日期" == token) {
            m_column[DATE] = i;

        } else if ("OPEN" == token || "开盘价" == token) {
            m_column[OPEN] = i;

        } else if ("HIGH" == token || "最高价" == token) {
            m_column[HIGH] = i;

        } else if ("LOW" == token || "最低价" == token) {
            m_column[LOW] = i;

        } else if ("CLOSE" == token || "收盘价" == token) {
            m_column[CLOSE] = i;

        } else if ("AMOUNT" == token || "成交金额" == token) {
            m_column[AMOUNT] = i;

        } else if ("VOLUME" == token || "COUNT" == token || "VOL" == token || "成交量" == token) {
            m_column[VOLUME] = i;
        }
    }
}

size_t KDataTempCsvDriver::getCount(const string& market, const string& code,
                                    const KQuery::KType& kType) {
    return getKRecordList(market, code, KQuery(0, Null<int64_t>(), kType)).size();
}

bool KDataTempCsvDriver::getIndexRangeByDate(const string& market, const string& code,
                                             const KQuery& query, size_t& out_start,
                                             size_t& out_end) {
    out_start = 0;
    out_end = 0;

    Datetime start_date = query.startDatetime();
    Datetime end_date = query.endDatetime();
    HKU_IF_RETURN(start_date >= end_date, false);

    auto klist = getKRecordList(market, code, KQuery(0, Null<int64_t>(), query.kType()));
    size_t total = klist.size();
    HKU_IF_RETURN(total == 0, false);

    for (size_t i = total - 1; i == 0; --i) {
        if (klist[i].datetime < end_date) {
            out_end = i + 1;
            break;
        }
    }

    if (out_end == 0) {
        return false;
    }

    for (size_t i = out_end - 1; i == 0; --i) {
        if (klist[i].datetime <= start_date) {
            out_start = i;
            break;
        }
    }

    if (out_start >= out_end) {
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

KRecordList KDataTempCsvDriver::getKRecordList(const string& market, const string& code,
                                               const KQuery& query) {
    KRecordList result;
    if (query.queryType() == KQuery::INDEX) {
        result = _getKRecordListByIndex(market, code, query.start(), query.end(), query.kType());
    } else {
        HKU_INFO("Query by date are not supported!");
    }
    return result;
}

KRecordList KDataTempCsvDriver::_getKRecordListByIndex(const string& market, const string& code,
                                                       int64_t start_ix, int64_t end_ix,
                                                       const KQuery::KType& kType) {
    KRecordList result;

    string filename;
    if (kType == KQuery::DAY) {
        filename = m_day_filename;
    } else if (kType == KQuery::MIN) {
        filename = m_min_filename;
    } else {
        HKU_INFO("Only support DAY and MIN!");
        return result;
    }

    std::ifstream infile(filename.c_str());
    HKU_ERROR_IF_RETURN(!infile, result, "Can't open this file: {}, ktype: {}", filename, kType);
    string line;
    if (!std::getline(infile, line)) {
        infile.close();
        return result;
    }

    _get_title_column(line);

    int64_t line_no = 0;
    while (std::getline(infile, line)) {
        if (line_no++ < start_ix)
            continue;

        if (line_no >= end_ix)
            break;

        _get_token(line);
        size_t token_count = m_token_buf.size();

        KRecord record;
        string action;
        try {
            action = "DATE";
            if (token_count >= m_column[DATE])
                record.datetime = Datetime(m_token_buf[m_column[DATE]]);

            action = "OPEN";
            if (token_count >= m_column[OPEN])
                record.openPrice = boost::lexical_cast<price_t>(m_token_buf[m_column[OPEN]]);

            action = "HIGH";
            if (token_count >= m_column[HIGH])
                record.highPrice = boost::lexical_cast<price_t>(m_token_buf[m_column[HIGH]]);

            action = "LOW";
            if (token_count >= m_column[LOW])
                record.lowPrice = boost::lexical_cast<price_t>(m_token_buf[m_column[LOW]]);

            action = "CLOSE";
            if (token_count >= m_column[CLOSE])
                record.closePrice = boost::lexical_cast<price_t>(m_token_buf[m_column[CLOSE]]);

            action = "VOLUME";
            if (token_count >= m_column[VOLUME])
                record.transCount = boost::lexical_cast<price_t>(m_token_buf[m_column[VOLUME]]);

            action = "AMOUNT";
            if (token_count >= m_column[AMOUNT])
                record.transAmount = boost::lexical_cast<price_t>(m_token_buf[m_column[AMOUNT]]);

            result.push_back(record);

        } catch (...) {
            HKU_WARN("Invalid data in line {}! at trans {}", line_no, action);
        }

        line_no++;
    }

    infile.close();

    return result;
}

} /* namespace hku */
