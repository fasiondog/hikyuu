/*
 * KData.cpp
 *
 *  Created on: 2013-1-20
 *      Author: fasiondog
 */

#include "KData.h"
#include "StockManager.h"
#include "KDataImp.h"
#include "indicator/crt/KDATA.h"
#include <fstream>

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const KData& kdata) {
    os << "KData{\n  size : " << kdata.size() << "\n  stock: " << kdata.getStock()
       << "\n  query: " << kdata.getQuery() << "\n}";
    return os;
}

string KData::toString() const {
    std::stringstream os;
    os << "KData{\n  size : " << size() << "\n  stock: " << getStock().toString()
       << ",\n  query: " << getQuery() << "\n}";
    return os.str();
}

KData::KData(const Stock& stock, const KQuery& query) {
    if (!stock.isNull()) {
        m_imp = KDataImpPtr(new KDataImp(stock, query));
    }
}

bool KData::operator==(const KData& thr) const {
    return this == &thr || m_imp == thr.m_imp ||
           (getStock() == thr.getStock() && getQuery() == thr.getQuery());
}

size_t KData::getPosInStock(Datetime datetime) const {
    size_t pos = getPos(datetime);
    return pos == Null<size_t>() ? Null<size_t>() : pos + startPos();
}

void KData::tocsv(const string& filename) {
    std::ofstream file(filename.c_str());
    HKU_ERROR_IF_RETURN(!file, void(), "Can't open file! ({})", filename);

    file << "date, open, high, low, close, amount, count" << std::endl;
    file.setf(std::ios_base::fixed);
    file.precision(4);
    string sep = ",";
    KRecord record;
    for (size_t i = 0; i < size(); ++i) {
        record = getKRecord(i);
        file << record.datetime << sep << record.openPrice << sep << record.highPrice << sep
             << record.lowPrice << sep << record.closePrice << sep << record.transAmount << sep
             << record.transCount << std::endl;
    }

    file.close();
}

Indicator KData::open() const {
    return OPEN(*this);
}

Indicator KData::close() const {
    return CLOSE(*this);
}

Indicator KData::low() const {
    return LOW(*this);
}

Indicator KData::high() const {
    return HIGH(*this);
}

Indicator KData::vol() const {
    return VOL(*this);
}

Indicator KData::amo() const {
    return AMO(*this);
}

KData HKU_API getKData(const string& market_code, const KQuery& query) {
    return StockManager::instance().getStock(market_code).getKData(query);
}

KData HKU_API getKData(const string& market_code, const Datetime& start, const Datetime& end,
                       const KQuery::KType& ktype, KQuery::RecoverType recoverType) {
    KQuery query(start, end, ktype, recoverType);
    return StockManager::instance().getStock(market_code).getKData(query);
}

KData HKU_API getKData(const string& market_code, int64_t start, int64_t end,
                       const KQuery::KType& ktype, KQuery::RecoverType recoverType) {
    KQuery query(start, end, ktype, recoverType);
    return StockManager::instance().getStock(market_code).getKData(query);
}

}  // namespace hku
