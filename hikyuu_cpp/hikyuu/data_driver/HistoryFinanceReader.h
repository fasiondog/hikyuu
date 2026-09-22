/*
 * HistoryFinanceReader.h
 *
 * Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef HISTORYFINANCEREADER_H_
#define HISTORYFINANCEREADER_H_

#include "../Stock.h"

namespace hku {

/**
 * Read the historical financial information
 * @ingroup DataDriver
 */
class HKU_API HistoryFinanceReader {
public:
    HistoryFinanceReader() = delete;
    explicit HistoryFinanceReader(const string& dir);
    virtual ~HistoryFinanceReader();

    PriceList getHistoryFinanceInfo(Datetime date, const string& market, const string& code);

private:
    string m_dir;  // The directory where the historical financial information files are stored
};

}  // namespace hku

#endif /* HISTORYFINANCEREADER_H_ */
