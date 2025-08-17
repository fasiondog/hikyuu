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

KRecord KData::ms_null_krecord;

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

KData::KData() : m_imp(make_shared<KDataImp>()) {}

KData::KData(const Stock& stock, const KQuery& query)
: m_imp(make_shared<KDataImp>(stock, query)) {}

bool KData::operator==(const KData& thr) const {
    return this == &thr || m_imp == thr.m_imp ||
           (getStock() == thr.getStock() && getQuery() == thr.getQuery()) ||
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
    const Stock& stk = getStock();
    if (stk.isNull()) {
        return KData();
    }

    const KQuery& query = getQuery();
    return KData(stk, KQueryByDate(start, end, query.kType(), query.recoverType()));
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

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> KData::toArrow() const noexcept {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    arrow::DoubleBuilder open_builder, high_builder, low_builder, close_builder, amount_builder,
      volume_builder;
    size_t total = this->size();
    HKU_ARROW_RETURN_NOT_OK(date_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(open_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(high_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(low_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(close_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(amount_builder.Reserve(total));
    HKU_ARROW_RETURN_NOT_OK(volume_builder.Reserve(total));

    const auto* ks = this->data();
    for (size_t i = 0; i < total; ++i) {
        HKU_ARROW_RETURN_NOT_OK(date_builder.Append(ks[i].datetime.timestamp() * 1000LL));
        HKU_ARROW_RETURN_NOT_OK(open_builder.Append(ks[i].openPrice));
        HKU_ARROW_RETURN_NOT_OK(high_builder.Append(ks[i].highPrice));
        HKU_ARROW_RETURN_NOT_OK(low_builder.Append(ks[i].lowPrice));
        HKU_ARROW_RETURN_NOT_OK(close_builder.Append(ks[i].closePrice));
        HKU_ARROW_RETURN_NOT_OK(amount_builder.Append(ks[i].transAmount));
        HKU_ARROW_RETURN_NOT_OK(volume_builder.Append(ks[i].transCount));
    }

    std::shared_ptr<arrow::Array> date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr,
      volume_arr;
    HKU_ARROW_RETURN_NOT_OK(date_builder.Finish(&date_arr));
    HKU_ARROW_RETURN_NOT_OK(open_builder.Finish(&open_arr));
    HKU_ARROW_RETURN_NOT_OK(high_builder.Finish(&high_arr));
    HKU_ARROW_RETURN_NOT_OK(low_builder.Finish(&low_arr));
    HKU_ARROW_RETURN_NOT_OK(close_builder.Finish(&close_arr));
    HKU_ARROW_RETURN_NOT_OK(amount_builder.Finish(&amount_arr));
    HKU_ARROW_RETURN_NOT_OK(volume_builder.Finish(&volume_arr));

    auto schema = arrow::schema({
      arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)),
      arrow::field("open", arrow::float64()),
      arrow::field("high", arrow::float64()),
      arrow::field("low", arrow::float64()),
      arrow::field("close", arrow::float64()),
      arrow::field("amount", arrow::float64()),
      arrow::field("volume", arrow::float64()),
    });

    return arrow::Table::Make(
      schema, {date_arr, open_arr, high_arr, low_arr, close_arr, amount_arr, volume_arr});
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
