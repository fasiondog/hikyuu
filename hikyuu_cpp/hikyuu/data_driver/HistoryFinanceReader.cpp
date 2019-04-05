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
#include "../StockTypeInfo.h"
#include "HistoryFinanceReader.h"

namespace hku {

HistoryFinanceReader::HistoryFinanceReader(const string& dir)
: m_dir(dir) {

}

HistoryFinanceReader::~HistoryFinanceReader() {

}

PriceList HistoryFinanceReader
::getHistoryFinanceInfo(Datetime date, const Stock& stock) {
    PriceList result;
    if (stock.type() != STOCKTYPE_A) {
        return result;
    }

    string filename(m_dir + "/gpcw" 
                   + boost::lexical_cast<string>(date.number() / 10000)
                   + ".dat");
    std::cout << filename << std::endl;
    std::ifstream file(filename.c_str(), std::ifstream::binary);
    if( !file ) {
        HKU_INFO("Can't found " << filename 
                 << " [HistoryFinanceReader::getHistoryFinanceInfo]");
        return result;
    }

    unsigned int report_date = 0;  
    unsigned short max_count = 0;
    unsigned long report_size = 0;

    char header_buf[20];
    file.read(header_buf, 20);
    memcpy(&report_date, header_buf + 2, 4);
    memcpy(&max_count, header_buf + 6, 2);
    memcpy(&report_size, header_buf + 12, 4);

    char stock_item_buf[11];
    char stock_code[7];
    unsigned long address = 0;
    for (int i = 0; i < max_count; i++) {
        file.read(stock_item_buf, 11);
        memcpy(stock_code, stock_item_buf, 6);
        stock_code[6] = '\0';
        if (string(stock_code) == stock.code()) {
            memcpy(&address, stock_item_buf + 7, 4);
            break;
        }
    }

    const int MAX_COL_NUM = 350;
    float result_buffer[MAX_COL_NUM];
    if (address != 0) {
        std::cout << address << std::endl;
        int report_fields_count = int(report_size / 4);
        if (report_fields_count >= MAX_COL_NUM) {
            HKU_WARN("Over MAX_COL_NUM! [HistoryFinanceReader::getHistoryFinanceInfo]");
            report_fields_count = MAX_COL_NUM;
            report_size = report_fields_count * 4;
        }
        file.seekg(address);
        file.read((char *)result_buffer, report_size);

        result.reserve(report_fields_count);
        price_t null_price = Null<price_t>();
        for (int i = 0; i < report_fields_count; i++) {
            if (result_buffer[i] == 0xf8f8f8f8) {
                result.push_back(null_price);
            } else {
                result.push_back(result_buffer[i]);
            }
        }
    }

    file.close();
    return result;
}

} /* namespace hku */



