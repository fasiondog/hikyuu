/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#pragma once

#include "KDataImp.h"

namespace hku {

class HKU_API KDataSharedBufferImp : public KDataImp {
public:
    KDataSharedBufferImp() = default;
    KDataSharedBufferImp(const Stock& stock, const KQuery& query);
    virtual ~KDataSharedBufferImp();

    virtual size_t getPos(const Datetime& datetime) const override;

    virtual const KRecord& getKRecord(size_t pos) const override;

    virtual const KRecord& front() const override {
        return m_data[0];
    }

    virtual const KRecord& back() const override {
        return m_data[m_size - 1];
    }

    virtual const KRecord* data() const override {
        return m_data;
    }

    virtual KRecord* data() override {
        return m_data;
    }

    virtual DatetimeList getDatetimeList() const override;

private:
    KRecord* m_data{nullptr};
};

} /* namespace hku */
