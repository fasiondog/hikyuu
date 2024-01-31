/*
 * KDataImp.h
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#pragma once
#ifndef KDATAIMP_H_
#define KDATAIMP_H_

#include "Stock.h"

namespace hku {

class HKU_API KDataImp {
public:
    KDataImp();
    KDataImp(const Stock& stock, const KQuery& query);
    virtual ~KDataImp();

    KQuery getQuery() const {
        return m_query;
    }

    Stock getStock() const {
        return m_stock;
    }

    KRecord getKRecord(size_t pos) const {
        return m_buffer[pos];
    }

    bool empty() const {
        return m_buffer.empty();
    }

    size_t size() {
        return m_buffer.size();
    }

    size_t startPos();
    size_t endPos();
    size_t lastPos();

    size_t getPos(const Datetime& datetime);

private:
    void _getPosInStock();
    void _recoverForward();
    void _recoverBackward();
    void _recoverEqualForward();
    void _recoverEqualBackward();
    void _recoverForUpDay();

private:
    KRecordList m_buffer;
    KQuery m_query;
    Stock m_stock;
    size_t m_start;
    size_t m_end;
    bool m_have_pos_in_stock;
};

typedef shared_ptr<KDataImp> KDataImpPtr;

} /* namespace hku */
#endif /* KDATAIMP_H_ */
