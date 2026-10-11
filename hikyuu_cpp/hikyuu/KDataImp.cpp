/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#include <functional>
#include "StockManager.h"
#include "KDataImp.h"
#include "KDataPrivatedBufferImp.h"

namespace hku {

KDataImp::KDataImp(const Stock& stock, const KQuery& query) : m_query(query), m_stock(stock) {}

KDataImp::~KDataImp() {}

KDataImpPtr KDataImp::getOtherFromSelf(const KQuery& query) const {
    return std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
}

} /* namespace hku */
