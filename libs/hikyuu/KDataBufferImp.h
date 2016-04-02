/*
 * KDataBufferImp.h
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#ifndef KDATABUFFERIMP_H_
#define KDATABUFFERIMP_H_

#include "KDataImp.h"

namespace hku {

class KDataBufferImp: public KDataImp {
public:
    KDataBufferImp();
    KDataBufferImp(const Stock& stock, const KQuery& query);
    virtual ~KDataBufferImp();

    virtual KRecord getKRecord(size_t pos) const {
        return m_buffer[pos];
    }

    virtual size_t getPos(const Datetime& datetime) const;

private:
    void _recoverForward();
    void _recoverBackward();
    void _recoverEqualForward();
    void _recoverEqualBackward();

private:
    KRecordList m_buffer;
};

} /* namespace hku */
#endif /* KDATABUFFERIMP_H_ */
