/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#include <functional>
#include "StockManager.h"
#include "KDataSharedBufferImp.h"

namespace hku {

KDataSharedBufferImp::KDataSharedBufferImp(const Stock& stock, const KQuery& query)
: KDataImp(stock, query) {
    bool sucess = m_stock.getIndexRange(query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
        return;
    }

    if (!m_stock.m_data || m_stock.m_data->pMutex.find(m_query.kType()) ==
                             m_stock.m_data->pMutex.end()) {
        m_start = 0;
        m_end = 0;
        return;
    }

    std::shared_lock<std::shared_mutex> lock(*(m_stock.m_data->pMutex[m_query.kType()]));
    m_buf = m_stock.m_data->pKData[m_query.kType()];
    if (!m_buf || m_start >= m_buf->size()) {
        m_start = 0;
        m_end = 0;
        return;
    }

    if (m_end > m_buf->size()) {
        m_end = m_buf->size();
    }
    m_size = m_end - m_start;
}

KDataSharedBufferImp::~KDataSharedBufferImp() {}

size_t KDataSharedBufferImp::getPos(const Datetime& datetime) const noexcept {
    return getKRecordPosByDatetime(dataptr(), size(), datetime);
}

const KRecord& KDataSharedBufferImp::getKRecord(size_t pos) const noexcept {
    return pos < m_size ? dataptr()[pos] : KRecord::NullKRecord;
}

DatetimeList KDataSharedBufferImp::getDatetimeList() const {
    DatetimeList result(m_size);
    const KRecord* p = dataptr();
    for (size_t i = 0; i < m_size; ++i) {
        result[i] = p[i].datetime;
    }
    return result;
}

KDataImpPtr KDataSharedBufferImp::getOtherFromSelf(const KQuery& query) const {
    return std::make_shared<KDataSharedBufferImp>(m_stock, query);
}

} /* namespace hku */
