/*
 * CsvKDataDriverImp.cpp
 *
 *  Created on: 2017年7月19日
 *      Author: fasiondog
 */

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "CsvKDataDriverImp.h"
#include "../../../utilities/util.h"
#include "../../../Log.h"

namespace hku {

CsvKDataDriverImp::CsvKDataDriverImp (
        const shared_ptr<IniParser>& config,
        const string& dirname,
        bool is_filename)
: KDataDriverImp(config), m_dirname(dirname), m_is_filename(is_filename) {
    for (int i = 0; i < LAST; ++i) {
        m_column[i] = Null<size_t>();
    }

    m_token_buf.reserve(LAST);
}

CsvKDataDriverImp::~CsvKDataDriverImp() {

}

string CsvKDataDriverImp::
_getFileName(const string& market, const string& code, KQuery::KType ktype) {
    string filename;
    if (m_is_filename) {
        filename = m_dirname;

    } else {

        switch (ktype) {
        case KQuery::MIN:
            filename = m_dirname + "\\" + market + "\\minline\\" + market + code + ".csv";
            break;

        case KQuery::MIN5:
        case KQuery::MIN15:
        case KQuery::MIN30:
        case KQuery::MIN60:
            filename = m_dirname + "\\" + market + "\\fzline\\" + market + code + ".csv";
            break;

        case KQuery::DAY:
        case KQuery::WEEK:
        case KQuery::MONTH:
        case KQuery::QUARTER:
        case KQuery::HALFYEAR:
        case KQuery::YEAR:
            filename = m_dirname + "\\" + market + "\\lday\\" + market + code + ".csv";
            break;

        default:
            break;
        }
    }

    return filename;
}

void CsvKDataDriverImp::_get_token(const string& line) {
    m_token_buf.clear();

    string token;
    size_t pos = 0, prepos = 0;
    pos = line.find(',', prepos);
    while (pos != line.npos) {
        token.assign(line, pos, pos - prepos);
        boost::trim(token);
        m_token_buf.push_back(token);
        prepos = pos + 1;
        pos = line.find(',', prepos);
    }

    if (prepos != pos) {
        boost::trim(token);
        m_token_buf.push_back(token);
    }
}

void CsvKDataDriverImp::_get_title_column(const string& line) {
    _get_token(line);

    int total = (int)m_token_buf.size();
    for (int i = 0; i < total; ++i) {
        string token = m_token_buf[i];
        boost::to_upper(token);

        if ("DATE" == token || "DATETIME" == token || HKU_STR("日期") == token) {
            m_column[DATE] = i;

        } else if ("OPEN" == token || HKU_STR("开盘价") == token) {
            m_column[OPEN] = i;

        } else if ("HIGH" == token || HKU_STR("最高价") == token) {
            m_column[HIGH] = i;

        } else if ("LOW" == token || HKU_STR("最低价") == token) {
            m_column[LOW] = i;

        } else if ("CLOSE" == token || HKU_STR("收盘价") == token) {
            m_column[CLOSE] = i;

        } else if ("AMOUNT" == token || HKU_STR("成交金额") == token) {
            m_column[AMOUNT] = i;

        } else if ("VOLUME" == token || "COUNT" == token || "VOL" == token
                || HKU_STR("成交") == token) {
            m_column[VOLUME] = i;
        }
    }
}


void CsvKDataDriverImp::loadKData(const string& market, const string& code,
        KQuery::KType kType, size_t start_ix, size_t end_ix,
        KRecordList* out_buffer) {
    string filename = _getFileName(market, code, kType);
    std::ifstream infile(filename.c_str());
    if (!infile) {
        HKU_ERROR("Can't open this file: " << filename
                << " [CsvKDataDriverImp::loadKData]");
        return;
    }

    string line;
    if (!std::getline(infile, line)) {
        infile.close();
        return;
    }

    _get_title_column(line);

    size_t line_no = 0;
    while (std::getline(infile, line)) {
        if (line_no < start_ix)
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
                record.transAmount = boost::lexical_cast<price_t>(m_token_buf[m_column[VOLUME]]);

            action = "AMOUNT";
            if (token_count >= m_column[AMOUNT])
                record.transCount = boost::lexical_cast<price_t>(m_token_buf[m_column[AMOUNT]]);

            out_buffer->push_back(record);

        } catch (...) {
            HKU_WARN("Invalid data in line " << line_no << "! at trans " << action
                    << " [CsvKDataDriverImp::loadKData]");
        }

        line_no++;
    }

    infile.close();
}

size_t
CsvKDataDriverImp::getCount(const string& market, const string& code,
         KQuery::KType kType) {
    string filename = _getFileName(market, code ,kType);
    std::ifstream infile(filename.c_str());
    if (!infile) {
        HKU_ERROR("Can't open this file: " << filename
                << " [CsvKDataDriverImp::getCount]");
        return 0;
    }

    string line;
    if (!std::getline(infile, line)) {
        infile.close();
        return 0;
    }

    _get_title_column(line);

    size_t line_no = 0;
    while (std::getline(infile, line)) {
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
                record.transAmount = boost::lexical_cast<price_t>(m_token_buf[m_column[VOLUME]]);

            action = "AMOUNT";
            if (token_count >= m_column[AMOUNT])
                record.transCount = boost::lexical_cast<price_t>(m_token_buf[m_column[AMOUNT]]);

            //out_buffer->push_back(record);

        } catch (...) {
            HKU_WARN("Invalid data in line " << line_no << "! at trans " << action
                    << " [CsvKDataDriverImp::getCount]");
        }

        line_no++;
    }

    infile.close();

    return line_no;
}

bool
CsvKDataDriverImp::getIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    string filename = _getFileName(market, code, query.kType());
    std::ifstream infile(filename.c_str());
    if (!infile) {
        HKU_ERROR("Can't open this file: " << filename
                << " [CsvKDataDriverImp::getIndexRangeByDate]");
        return false;
    }

    string line;
    if (!std::getline(infile, line)) {
        infile.close();
        return false;
    }

    _get_title_column(line);

    Datetime startDate = query.startDatetime();
    Datetime endDate = query.endDatetime();

    Datetime currentDate;
    size_t line_no = 0;
    while (std::getline(infile, line)) {
        _get_token(line);
        size_t token_count = m_token_buf.size();

        string action;
        try {
            action = "DATE";
            if (token_count >= m_column[DATE]) {
                currentDate = Datetime(m_token_buf[m_column[DATE]]);
                if (out_start != 0 && currentDate >= startDate ) {
                    out_start = line_no;
                }
                if (out_end != 0 && currentDate > endDate) {
                    out_end = line_no;
                }
            }

        } catch (...) {
            HKU_WARN("Invalid data in line " << line_no << "! at trans " << action
                    << " [CsvKDataDriverImp::getIndexRangeByDate]");
        }

        line_no++;
    }

    infile.close();

    if (out_start !=0 && out_end == 0) {
        out_end = line_no;
    }

    return true;
}

KRecord
CsvKDataDriverImp::getKRecord(const string& market, const string& code,
          size_t pos, KQuery::KType kType) {
    KRecord record;
    string filename = _getFileName(market, code, kType);
    std::ifstream infile(filename.c_str());
    if (!infile) {
        HKU_ERROR("Can't open this file: " << filename
                << " [CsvKDataDriverImp::getKRecord]");
        return record;
    }

    string line;
    if (!std::getline(infile, line)) {
        infile.close();
        return record;
    }

    _get_title_column(line);

    size_t line_no = 0;
    while (std::getline(infile, line)) {
        _get_token(line);
        size_t token_count = m_token_buf.size();


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
                record.transAmount = boost::lexical_cast<price_t>(m_token_buf[m_column[VOLUME]]);

            action = "AMOUNT";
            if (token_count >= m_column[AMOUNT])
                record.transCount = boost::lexical_cast<price_t>(m_token_buf[m_column[AMOUNT]]);

            //out_buffer->push_back(record);

        } catch (...) {
            HKU_WARN("Invalid data in line " << line_no << "! at trans " << action
                    << " [CsvKDataDriverImp::getKRecord]");
        }

        if (line_no == pos) {
            break;
        }

        line_no++;
    }

    infile.close();

    return record;
}

} /* namespace hku */
