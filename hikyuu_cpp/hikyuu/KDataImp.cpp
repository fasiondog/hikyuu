/*
 * KDataImp.cpp
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include "StockManager.h"
#include "KDataImp.h"

namespace hku {

KDataImp::KDataImp() : m_start(0), m_end(0) {}

KDataImp::KDataImp(const Stock& stock, const KQuery& query)
: m_query(query), m_stock(stock), m_start(0), m_end(0), m_have_pos_in_stock(false) {
    if (m_stock.isNull()) {
        return;
    }

    /*bool sucess = m_stock.getIndexRange(query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
    }*/
}

KDataImp::~KDataImp() {}

bool KDataImp::empty() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_start == m_end ? true : false;
}

size_t KDataImp::startPos() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_start;
}

size_t KDataImp::endPos() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_end;
}

size_t KDataImp::lastPos() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_end == 0 ? 0 : m_end - 1;
}

void KDataImp::_getPosInStock() {
    bool sucess = m_stock.getIndexRange(m_query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
    }
    m_have_pos_in_stock = true;
}

KRecord KDataImp::getKRecord(size_t pos) {
    return empty() ? Null<KRecord>() : m_stock.getKRecord(startPos() + pos, m_query.kType());
}

size_t KDataImp::getPos(const Datetime& datetime) {
    KRecord null_record;
    if (empty()) {
        return Null<size_t>();
    }

    size_t mid, low = 0, high = size() - 1;
    while (low <= high) {
        if (datetime > getKRecord(high).datetime) {
            mid = high + 1;
            break;
        }

        if (getKRecord(low).datetime >= datetime) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if (datetime > getKRecord(mid).datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (mid >= size()) {
        return Null<size_t>();
    }

    KRecord tmp = getKRecord(mid);
    return tmp.datetime == datetime ? mid : Null<size_t>();
}

} /* namespace hku */
