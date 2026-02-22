/*
 * KData.cpp
 *
 *  Created on: 2013-1-20
 *      Author: fasiondog
 */

#include "KData.h"
#include "StockManager.h"
#include "KDataSharedBufferImp.h"
#include "KDataPrivatedBufferImp.h"
#include "indicator/crt/KDATA.h"
#include "plugin/hkuextra.h"
#include <fstream>

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const KData& kdata) {
    os << "KData{\n  size : " << kdata.size() << "\n  stock: " << kdata.getStock()
       << "\n  query: " << kdata.getQuery();
    if (kdata.size() == 1) {
        os << "\n  " << kdata.getKRecord(0);
    } else if (kdata.size() > 1) {
        os << "\n  first: " << kdata.getKRecord(0);
        os << "\n  last: " << kdata.getKRecord(kdata.size() - 1);
    }
    os << "\n}";
    return os;
}

string KData::toString() const {
    std::stringstream os;
    os << "KData{\n  size : " << size() << "\n  stock: " << getStock().toString()
       << ",\n  query: " << getQuery();
    if (size() == 1) {
        os << "\n  " << getKRecord(0);
    } else if (size() > 1) {
        os << "\n  first: " << getKRecord(0);
        os << "\n  last: " << getKRecord(size() - 1);
    }
    os << "\n}";
    return os.str();
}

KData::KData() : m_imp(get_null_kdata_imp()) {}

KData::KData(KDataImpPtr imp) : m_imp(imp ? imp : get_null_kdata_imp()) {}

KData::KData(const Stock& stock, const KQuery& query) {
    // 在重加载或setKDateList时，已存在KData存在数据无效风险（但无内存访问问题)
    if (stock.isNull()) {
        m_imp = get_null_kdata_imp();
        return;
    }

    if (stock.isPreload(query.kType()) && !stock.isBuffer(query.kType())) {
        stock.loadKDataToBuffer(query.kType());
    }

    // 非预加载也可能被主动加载至缓存
    if (query.recoverType() == KQuery::NO_RECOVER && stock.isBuffer(query.kType())) {
        // 当Stock已缓存了该类型的K线数据，且不进行复权
        m_imp = make_shared<KDataSharedBufferImp>(stock, query);
        return;
    }

    m_imp = getKDataImp(stock, query);
}

bool KData::operator==(const KData& thr) const noexcept {
    return this == &thr || m_imp == thr.m_imp ||
           (getStock() == thr.getStock() && getQuery() == thr.getQuery() && size() == thr.size()) ||
           (getStock().isNull() && thr.getStock().isNull());
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

KData KData::getKData(const Datetime& start, const Datetime& end) const {
    const auto& self_query = getQuery();
    return getKData(KQueryByDate(start, end, self_query.kType(), self_query.recoverType()));
}

KData KData::getKData(const KQuery& query) const {
    KData ret;
    const Stock& stk = getStock();
    HKU_IF_RETURN(stk.isNull(), ret);

    if (stk.isPreload(query.kType()) && !stk.isBuffer(query.kType())) {
        stk.loadKDataToBuffer(query.kType());
    }

    if (query.recoverType() == KQuery::NO_RECOVER && stk.isBuffer(query.kType())) {
        ret = KData(stk, query);
        return ret;
    }

    const auto& self_query = getQuery();
    if (empty() || self_query.recoverType() != query.recoverType() ||
        query.kType() != self_query.kType() || self_query.recoverType() == KQuery::FORWARD ||
        self_query.recoverType() == KQuery::EQUAL_FORWARD) {
        ret = KData(stk, query);
        return ret;
    }

    if (query == self_query) {
        ret.m_imp = m_imp;
        return ret;
    }

    auto imp = m_imp->getOtherFromSelf(query);
    ret.m_imp = std::move(imp);
    return ret;
}

KData KData::getSubKData(int64_t start, int64_t end) const {
    int64_t total = static_cast<int64_t>(size());
    size_t startix, endix;
    if (start < 0) {
        startix = start < -total ? 0 : total + start;
    } else {
        startix = start;
    }

    if (end == Null<int64_t>()) {
        endix = total;
    } else if (end < 0) {
        endix = end < -total ? 0 : total + end;
    } else {
        endix = end;
    }

    size_t startpos = startPos();

    const auto& self_query = getQuery();
    KQuery query =
      KQuery(startix + startpos, endix + startpos, self_query.kType(), self_query.recoverType());

    return KData(getStock(), query);
}

KQuery KData::getOtherQueryByDate(const Datetime& start_datetime, const Datetime& end_datetime,
                                  const KQuery::KType& ktype) const {
    const KQuery& query = getQuery();
    if (getStock().isNull()) {
        return KQueryByDate(Null<Datetime>(), Null<Datetime>(), ktype, query.recoverType());
    }

    if (empty() && query.queryType() == KQuery::INDEX) {
        return KQuery(Null<Datetime>(), Null<Datetime>(), ktype, query.recoverType());
    }

    Datetime end;
    if ((query.queryType() == KQuery::INDEX && query.end() == Null<int64_t>()) ||
        (query.queryType() == KQuery::DATE && query.endDatetime() == Null<Datetime>())) {
        end = Null<Datetime>();
    } else {
        end = this->back().datetime;
        if (end_datetime != Null<Datetime>() && end_datetime < end) {
            end = end_datetime;
        }
    }

    Datetime start = this->front().datetime;
    if (start_datetime > start) {
        start = start_datetime;
    }

    auto day_ktype_seconds = KQuery::getKTypeInSeconds(KQuery::DAY);
    // 从日线及以上转日线以下
    if (KQuery::getKTypeInSeconds(ktype) < day_ktype_seconds &&
        KQuery::getKTypeInSeconds(query.kType()) >= day_ktype_seconds) {
        if (end != Null<Datetime>()) {
            end = end.endOfDay();
        }
    }

    // 从日线以下转日线及以上
    if (KQuery::getKTypeInSeconds(ktype) >= day_ktype_seconds &&
        KQuery::getKTypeInSeconds(query.kType()) < day_ktype_seconds) {
        start = start.startOfDay();
    }

    return KQuery(start, end, ktype, query.recoverType());
}

KData KData::getKData(const KQuery::KType& ktype) const {
    KData ret;
    const Stock& stk = getStock();
    HKU_IF_RETURN(stk.isNull(), ret);
    ret = stk.getKData(getOtherQueryByDate(front().datetime, back().datetime, ktype));
    return ret;
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
