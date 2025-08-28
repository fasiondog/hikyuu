/*
 * KDataPrivatedBufferImp.h
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#pragma once

#include "KDataImp.h"

namespace hku {

class KDataPrivatedBufferImp : public KDataImp {
public:
    KDataPrivatedBufferImp();
    KDataPrivatedBufferImp(const Stock& stock, const KQuery& query);
    virtual ~KDataPrivatedBufferImp();

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
    void _getPosInStock();
    void _recoverForward();
    void _recoverBackward();
    void _recoverEqualForward();
    void _recoverEqualBackward();
    void _recoverForUpDay();

private:
    KRecordList m_buffer;
};

typedef shared_ptr<KDataPrivatedBufferImp> KDataPrivatedBufferImpPtr;

} /* namespace hku */
