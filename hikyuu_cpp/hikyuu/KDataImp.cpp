/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#include <functional>
#include "StockManager.h"
#include "KDataImp.h"

namespace hku {

KDataImp::KDataImp(const Stock& stock, const KQuery& query) : m_query(query), m_stock(stock) {
    if (m_stock.isNull()) {
        return;
    }

    bool sucess = m_stock.getIndexRange(query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
        return;
    }

    m_size = m_end - m_start;
}

KDataImp::~KDataImp() {}

} /* namespace hku */
