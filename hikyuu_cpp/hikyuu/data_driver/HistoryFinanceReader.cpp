/*
 * HistoryFinanceReader.cpp
 * 
 * Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include <fstream>
#include <boost/lexical_cast.hpp>
#include "HistoryFinanceReader.h"

namespace hku {

HistoryFinanceReader::HistoryFinanceReader(const string& dir)
: m_dir(dir) {

}

HistoryFinanceReader::~HistoryFinanceReader() {

}

PriceList HistoryFinanceReader
::getHistoryFinanceInfo(Datetime date, 
        const string& market, const string& code) {
    PriceList result;
    string filename(m_dir + "\\gpcw" 
                   + boost::lexical_cast<string>(date.number() / 10000)
                   + ".dat");
    std::cout << filename << std::endl;
    std::ifstream file(filename.c_str(), std::ifstream::binary);
    if( !file ) {
        std::cout << "error!" << std::endl;
        return result;
    }

    short unused = 0; 
    unsigned int report_date = 0;  
    unsigned short max_count = 0;
    unsigned long report_fields_count = 0;

    char header_buf[20];
    file.read(header_buf, 20);
    memcpy(&report_date, header_buf + 2, 4);
    memcpy(&max_count, header_buf + 6, 2);
    memcpy(&report_fields_count, header_buf + 12, 4);

    std::cout << report_date << " " << max_count << " " << report_fields_count << std::endl;

    file.close();
    return result;
}

} /* namespace hku */



