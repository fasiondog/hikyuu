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

KDataImp::KDataImp(const Stock& stock, const KQuery& query) : m_query(query), m_stock(stock) {}

KDataImp::~KDataImp() {}

} /* namespace hku */
