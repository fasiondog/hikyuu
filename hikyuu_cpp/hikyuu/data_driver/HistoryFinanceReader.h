/*
 * HistoryFinanceReader.h
 * 
 * Copyright (c) 2019 fasiondog
 *  
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#ifndef HISTORYFINANCEREADER_H_
#define HISTORYFINANCEREADER_H_

#include "../Stock.h"

namespace hku {

/**
 * 读取历史财务信息
 */
class HKU_API HistoryFinanceReader {

public:
    HistoryFinanceReader() = delete;
    HistoryFinanceReader(const string& dir);
    virtual ~HistoryFinanceReader();

    PriceList getHistoryFinanceInfo(Datetime date, 
                const string& market, const string& code);

private:
    string m_dir;  //历史财务信息文件存放目录
};


} /* namespace */

#endif /* HISTORYFINANCEREADER_H_ */
