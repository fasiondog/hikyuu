/*
 * KData.cpp
 *
 *  Created on: 2013-1-20
 *      Author: fasiondog
 */

#include "KData.h"
#include "StockManager.h"
#include "KDataBufferImp.h"
#include <fstream>

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os, const KData& kdata) {
    os << "KData{\n  size : " << kdata.size() << "\n  stock: " << kdata.getStock() << ",\n  query: "
       << kdata.getQuery() << "\n }";
    return os;
}


string KData::toString() const {
    std::stringstream os;
    os << "KData{\n  size : " << size() << "\n  stock: "
       << getStock().toString() << ",\n  query: "
       << getQuery() << "\n }";
    return os.str();
}


KData::KData(const Stock& stock, const KQuery& query) {
    if (stock.isNull() || query.kType() >= KQuery::INVALID_KTYPE) {
        return;
    }
    if (query.kType() < KQuery::DAY) {
        //m_imp = KDataImpPtr(new KDataImp(stock, query));
    	m_imp = KDataImpPtr(new KDataBufferImp(stock, query));

    } else if (query.kType() == KQuery::DAY) {
        m_imp = KDataImpPtr(new KDataBufferImp(stock, query));

    } else {
        //日线以上不支持复权
        if (query.queryType() == KQuery::INDEX) {
            KQuery new_query(query.start(), query.end(), query.kType(),
                    KQuery::NO_RECOVER);
            m_imp = KDataImpPtr(new KDataBufferImp(stock, new_query));
        } else {
            KQuery new_query = KQueryByDate(query.startDatetime(),
                    query.endDatetime(), query.kType(), KQuery::NO_RECOVER);
            m_imp = KDataImpPtr(new KDataBufferImp(stock, new_query));
        }
    }
#if 0
    if (stock.isBuffer(query.kType())
            && query.recoverType() == KQuery::NO_RECOVER) {
        //当Stock已缓存了该类型的K线数据，且不进行复权
        m_imp = KDataImpPtr(new KDataImp(stock, query));
    } else {
        m_imp = KDataImpPtr(new KDataBufferImp(stock, query));
    }
#endif
}


void KData::tocsv(const string& filename) {
    std::ofstream file(filename.c_str());
    if (!file) {
        HKU_ERROR("Can't open file! (" << filename << ") [KData::tocsv");
        return;
    }

    file << "date, open, high, low, close, amount, count" << std::endl;
    file.setf(std::ios_base::fixed);
    file.precision(4);
    string sep = ",";
    KRecord record;
    for (size_t i = 0; i < size(); ++i) {
        record = getKRecord(i);
        file << record.datetime << sep << record.openPrice << sep
                << record.highPrice << sep << record.lowPrice << sep
                << record.closePrice << sep << record.transAmount << sep
                << record.transCount << std::endl;
    }

    file.close();
}


} /* namespace */


