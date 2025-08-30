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
    virtual ~KDataPrivatedBufferImp();

    virtual bool empty() const override {
        return m_buffer.empty();
    }

    virtual size_t size() const override {
        return m_buffer.size();
    }

    virtual size_t startPos() const override;
    virtual size_t endPos() const override;
    virtual size_t lastPos() const override;

    virtual size_t getPos(const Datetime& datetime) const override;

    virtual const KRecord& getKRecord(size_t pos) const override {
        return m_buffer[pos];
    }

    virtual const KRecord& front() const override {
        return m_buffer.front();
    }

    virtual const KRecord& back() const override {
        return m_buffer.back();
    }

    virtual const KRecord* data() const override {
        return m_buffer.data();
    }

    virtual KRecord* data() override {
        return m_buffer.data();
    }

    virtual DatetimeList getDatetimeList() const override;

private:
    void _getPosInStock() const;
    void _recoverForward();
    void _recoverBackward();
    void _recoverEqualForward();
    void _recoverEqualBackward();
    void _recoverForUpDay();

private:
    KRecordList m_buffer;
    mutable size_t m_start{0};
    mutable size_t m_end{0};
    mutable bool m_have_pos_in_stock{false};
};

typedef shared_ptr<KDataPrivatedBufferImp> KDataPrivatedBufferImpPtr;

} /* namespace hku */
