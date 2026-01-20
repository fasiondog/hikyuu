/*
 * KDataPrivatedBufferImp.h
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#pragma once

#include "KDataImp.h"

namespace hku {

class HKU_API KDataPrivatedBufferImp : public KDataImp {
public:
    KDataPrivatedBufferImp();
    KDataPrivatedBufferImp(const Stock& stock, const KQuery& query);
    virtual ~KDataPrivatedBufferImp() override;

    virtual bool empty() const noexcept override {
        return m_buffer.empty();
    }

    virtual size_t size() const noexcept override {
        return m_buffer.size();
    }

    virtual size_t startPos() const override;
    virtual size_t endPos() const override;
    virtual size_t lastPos() const override;

    virtual size_t getPos(const Datetime& datetime) const noexcept override;

    virtual const KRecord& getKRecord(size_t pos) const noexcept override {
        return m_buffer[pos];
    }

    virtual const KRecord& front() const override {
        return m_buffer.front();
    }

    virtual const KRecord& back() const override {
        return m_buffer.back();
    }

    virtual const KRecord* data() const noexcept override {
        return m_buffer.data();
    }

    virtual KRecord* data() noexcept override {
        return m_buffer.data();
    }

    virtual DatetimeList getDatetimeList() const override;

    virtual KDataImpPtr getOtherFromSelf(const KQuery& query) const override;

private:
    void _getPosInStock() const;
    void _recover();
    void _recoverForward();
    void _recoverBackward();
    void _recoverEqualForward();
    void _recoverEqualBackward();
    void _recoverForUpDay();

    KDataImpPtr _getOtherFromSelfByIndex(const KQuery& query) const;
    KDataImpPtr _getOtherFromSelfByDate(const KQuery& query) const;

private:
    KRecordList m_buffer;
    mutable size_t m_start{0};
    mutable size_t m_end{0};
    mutable bool m_have_pos_in_stock{false};
};

// typedef shared_ptr<KDataPrivatedBufferImp> KDataPrivatedBufferImpPtr;

} /* namespace hku */
