/*
 * KDataImp.cpp
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include "StockManager.h"
#include "KDataImp.h"

namespace hku {

KDataImp::KDataImp(): m_start(0), m_end(0) {

}


KDataImp::
KDataImp(const Stock& stock, const KQuery& query)
: m_query(query), m_stock(stock), m_start(0), m_end(0){
    if (m_stock.isNull()) {
        return;
    }

    bool sucess = m_stock.getIndexRange(query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
        return;
    }
}


KDataImp::~KDataImp() {

}


KRecord KDataImp::getKRecord(size_t pos) const {
    return empty() ? Null<KRecord>()
            : m_stock.getKRecord(m_start + pos, m_query.kType());
}

KRecord KDataImp::getKRecordByDate(const Datetime& datetime) const {
    KRecord null_record;
    if (empty()) {
        return null_record;
    }

    size_t mid, low = 0, high = size() - 1;
    while(low <= high) {
        if(datetime > getKRecord(high).datetime) {
            mid = high + 1;
            break;
        }

        if(getKRecord(low).datetime >= datetime) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if(datetime > getKRecord(mid).datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if(mid >= size()) {
        return null_record;
    }

    KRecord tmp = getKRecord(mid);
    return tmp.datetime == datetime ? tmp : null_record;
}


} /* namespace hku */
