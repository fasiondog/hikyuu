/*
 * KDataImp.h
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#ifndef KDATAIMP_H_
#define KDATAIMP_H_

#include "Stock.h"

namespace hku {

class KDataImp {
public:
    KDataImp();
    KDataImp(const Stock& stock, const KQuery& query);
    virtual ~KDataImp();

    bool empty() const {
        return m_start == m_end ? true : false;
    }

    size_t size() const { return m_end - m_start; }
    KQuery getQuery() const { return m_query; }
    Stock getStock() const { return m_stock; }
    size_t startPos() const { return m_start; }
    size_t endPos() const { return m_end; }

    size_t lastPos() const {
        return m_end == 0 ? 0 : m_end - 1;
    }

    virtual KRecord getKRecord(size_t pos) const;

    virtual size_t getPos(const Datetime& datetime) const;

protected:
    KQuery m_query;
    Stock  m_stock;
    size_t m_start;
    size_t m_end;
};

typedef shared_ptr<KDataImp> KDataImpPtr;


} /* namespace hku */
#endif /* KDATAIMP_H_ */
