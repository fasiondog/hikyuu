/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#include "HistoryFinanceInfo.h"

namespace hku {

HistoryFinanceInfo& HistoryFinanceInfo::operator=(const HistoryFinanceInfo& other) {
    HKU_IF_RETURN(this == &other, *this);
    fileDate = other.fileDate;
    reportDate = other.reportDate;
    values = other.values;
    return *this;
}

HistoryFinanceInfo& HistoryFinanceInfo::operator=(HistoryFinanceInfo&& other) {
    HKU_IF_RETURN(this == &other, *this);
    fileDate = std::move(other.fileDate);
    reportDate = std::move(other.reportDate);
    values = std::move(other.values);
    return *this;
}

}  // namespace hku