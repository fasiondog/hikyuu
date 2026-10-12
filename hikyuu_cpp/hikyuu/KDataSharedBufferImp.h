/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#pragma once

#include "KDataImp.h"

namespace hku {

/**
 * Zero-copy KData view over the buffer cached by the Stock (NO_RECOVER only).
 * @note Holds a shared_ptr copy of the buffer (a snapshot), safe against a concurrent release;
 *       the data pointer is resolved on every access
 */
class HKU_API KDataSharedBufferImp : public KDataImp {
public:
    KDataSharedBufferImp() = default;
    KDataSharedBufferImp(const Stock& stock, const KQuery& query);
    virtual ~KDataSharedBufferImp() override;

    virtual bool empty() const noexcept override {
        return m_size == 0;
    }

    virtual size_t size() const noexcept override {
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

    virtual size_t getPos(const Datetime& datetime) const noexcept override;

    virtual const KRecord& getKRecord(size_t pos) const noexcept override;

    virtual const KRecord& front() const override {
        return (m_size == 0 || !m_buf) ? KRecord::NullKRecord : m_buf->data()[m_start];
    }

    virtual const KRecord& back() const override {
        return (m_size == 0 || !m_buf) ? KRecord::NullKRecord : m_buf->data()[m_end - 1];
    }

    virtual const KRecord* data() const noexcept override {
        return dataptr();
    }

    virtual DatetimeList getDatetimeList() const override;

    virtual KDataImpPtr getOtherFromSelf(const KQuery& query) const override;

private:
    KRecord* dataptr() const noexcept {
        return m_buf ? m_buf->data() + m_start : nullptr;
    }

private:
    size_t m_start{0};
    size_t m_end{0};
    size_t m_size{0};
    std::shared_ptr<KRecordList> m_buf{nullptr};  // Pins the stock buffer snapshot
};

} /* namespace hku */
