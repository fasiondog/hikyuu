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

    virtual bool empty() const override {
        return m_size == 0;
    }

    virtual size_t size() const override {
        return m_size;
    }

    virtual size_t startPos() const override {
        return m_start;
    }

    virtual size_t endPos() const override {
        return m_end;
    }

    virtual size_t lastPos() const override {
        return m_end == 0 ? 0 : m_end - 1;
    }

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
    size_t m_start{0};
    size_t m_end{0};
    size_t m_size{0};
    KRecord* m_data{nullptr};
};

} /* namespace hku */
