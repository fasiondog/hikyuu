/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#pragma once

#include "Stock.h"

namespace hku {

class HKU_API KDataImp {
public:
    KDataImp() = default;
    KDataImp(const Stock& stock, const KQuery& query);
    virtual ~KDataImp();

    const KQuery& getQuery() const {
        return m_query;
    }

    const Stock& getStock() const {
        return m_stock;
    }

    bool empty() const {
        return m_size == 0;
    }

    size_t size() const {
        return m_size;
    }

    size_t startPos() const {
        return m_start;
    }

    size_t endPos() const {
        return m_end;
    }

    size_t lastPos() const {
        return m_end == 0 ? 0 : m_end - 1;
    }

    virtual size_t getPos(const Datetime& datetime) const {
        return Null<size_t>();
    }

    virtual const KRecord& getKRecord(size_t pos) const {
        return KRecord::NullKRecord;
    }

    virtual const KRecord& front() const {
        return KRecord::NullKRecord;
    }

    virtual const KRecord& back() const {
        return KRecord::NullKRecord;
    }

    virtual const KRecord* data() const {
        return nullptr;
    }

    virtual KRecord* data() {
        return nullptr;
    }

    virtual DatetimeList getDatetimeList() const {
        return DatetimeList();
    }

protected:
    KQuery m_query;
    Stock m_stock;
    size_t m_start{0};
    size_t m_end{0};
    size_t m_size{0};
};

typedef shared_ptr<KDataImp> KDataImpPtr;

} /* namespace hku */
