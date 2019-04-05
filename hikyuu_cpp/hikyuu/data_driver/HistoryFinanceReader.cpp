/*
 * HistoryFinanceReader.cpp
 * 
 * Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include <stdio.h>
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
    string funcname(" [HistoryFinanceReader::getHistoryFinanceInfo]");
    PriceList result;

    string filename(m_dir + "/gpcw" 
                   + boost::lexical_cast<string>(date.number() / 10000)
                   + ".dat");
    FILE *fp = fopen(filename.c_str(), "rb");
    if (NULL == fp) {
        HKU_INFO("Can't found " << filename << funcname);
        return result;
    }

    unsigned int report_date = 0;  
    unsigned short max_count = 0;
    unsigned long report_size = 0;
    
    char header_buf[20];
    if (!fread(header_buf, 1, 20, fp)) {
        HKU_ERROR("read data failed! " << filename << funcname);
        return result;
    }

    memcpy(&report_date, header_buf + 2, 4);
    memcpy(&max_count, header_buf + 6, 2);
    memcpy(&report_size, header_buf + 12, 4);

    char stock_code[7];
    unsigned long address = 0;
    for (int i = 0; i < max_count; i++) {
        if (!fread(stock_code, 1, 7, fp)) {
            HKU_ERROR("read stock_code failed! " << filename << funcname);
            return result;
        }

        if (!fread(&address, sizeof(unsigned long), 1, fp)) {
            HKU_ERROR("read stock_item address failed! " << filename << funcname);
            return result;
        }

        stock_code[6] = '\0';
        if (strcmp(stock_code, code.c_str()) == 0) {
            break;
        }
    }

    const int MAX_COL_NUM = 350;
    float result_buffer[MAX_COL_NUM];
    if (address != 0) {
        int report_fields_count = int(report_size / 4);
        if (report_fields_count >= MAX_COL_NUM) {
            HKU_WARN("Over MAX_COL_NUM! [HistoryFinanceReader::getHistoryFinanceInfo]");
            report_fields_count = MAX_COL_NUM;
        }
        
        fseek(fp, address, SEEK_SET);

        if (!fread((char *)result_buffer, sizeof(float), report_fields_count, fp)) {
            HKU_ERROR("read col data failed!" << filename << funcname);
            return result;
        }

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

    fclose(fp);
    return result;
}

} /* namespace hku */



