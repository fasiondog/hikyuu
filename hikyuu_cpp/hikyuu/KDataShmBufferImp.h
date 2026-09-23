/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-04
 *      Author: fasiondog
 */

#pragma once

#include "KDataImp.h"

namespace hku {

struct KRecordView;  // Defined in data_driver/KDataDriver.h

/**
 * Zero-copy K-line view of the client shared memory
 * @details In client mode a NO_RECOVER query directly reads the shared memory snapshot published by
 * the main process through a raw pointer view, without copying the records (all the clients share
 * the same physical page), which aligns with the zero-copy semantics of KDataSharedBufferImp in the
 * main process. The view pointer is pinned by m_pin (which holds a type-erased KDataShmReader), so
 * the old mapping stays alive after the epoch is replaced. It is constructed successfully through
 * create() only when the driver supports the view (a client IPC proxy driver with the snapshot
 * covered and no reverse scaling needed), otherwise a null pointer is returned and the upper layer
 * falls back to the KDataPrivatedBufferImp copy path.
 * @note Read-only mapping: the writable overload of data() returns a const_cast pointer only for
 *       interface compatibility, a write will crash, which is the same kind of risk as the "use
 * with caution (forced data adjustment)" contract of KDataSharedBufferImp in the main process.
 * @ingroup StockManage
 */
class HKU_API KDataShmBufferImp : public KDataImp {
public:
    /**
     * Factory: try to construct the shared memory zero-copy view in client mode
     * @param stock the target security
     * @param query the query condition (should be NO_RECOVER, guaranteed by the caller)
     * @return the view imp on success; nullptr when it does not apply (not a client / not covered /
     *         reverse scaling needed / invalid range)
     */
    static KDataImpPtr create(const Stock& stock, const KQuery& query);

    virtual ~KDataShmBufferImp() override;

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

    virtual const KRecord& getKRecord(size_t pos) const noexcept override {
        return pos < m_size ? m_data[pos] : KRecord::NullKRecord;
    }

    virtual const KRecord& front() const override {
        return m_data[0];
    }

    virtual const KRecord& back() const override {
        return m_data[m_size - 1];
    }

    virtual const KRecord* data() const noexcept override {
        return m_data;
    }

    virtual KRecord* data() noexcept override {
        // Read-only shared memory mapping, a write will crash; it exists only for compatibility
        // with the KDataImp interface (see the class comment)
        return const_cast<KRecord*>(m_data);
    }

    virtual DatetimeList getDatetimeList() const override;

    virtual KDataImpPtr getOtherFromSelf(const KQuery& query) const override;

private:
    KDataShmBufferImp(const Stock& stock, const KQuery& query, const KRecordView& view,
                      size_t start);

    size_t m_start{0};
    size_t m_end{0};
    size_t m_size{0};
    const KRecord* m_data{nullptr};
    std::shared_ptr<void> m_pin;  ///< Keeps the shm mapping alive (type-erased KDataShmReaderPtr)
};

} /* namespace hku */
