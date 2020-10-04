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

class KDataImp {
public:
    KDataImp();
    KDataImp(const Stock& stock, const KQuery& query);
    virtual ~KDataImp();

    bool empty();

    size_t size() const {
        return m_end - m_start;
    }

    KQuery getQuery() const {
        return m_query;
    }

    Stock getStock() const {
        return m_stock;
    }

    size_t startPos();
    size_t endPos();
    size_t lastPos();

    virtual KRecord getKRecord(size_t pos);

    virtual size_t getPos(const Datetime& datetime);

protected:
    void _getPosInStock();

protected:
    KQuery m_query;
    Stock m_stock;
    size_t m_start;
    size_t m_end;
    bool m_have_pos_in_stock;
};

typedef shared_ptr<KDataImp> KDataImpPtr;

} /* namespace hku */
#endif /* KDATAIMP_H_ */
