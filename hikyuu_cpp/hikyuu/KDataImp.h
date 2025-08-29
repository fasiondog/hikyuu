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

    virtual bool empty() const {
        return true;
    }

    virtual size_t size() const {
        return 0;
    }

    virtual size_t startPos() const {
        return 0;
    }

    virtual size_t endPos() const {
        return 0;
    }

    virtual size_t lastPos() const {
        return 0;
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
};

typedef shared_ptr<KDataImp> KDataImpPtr;

} /* namespace hku */
