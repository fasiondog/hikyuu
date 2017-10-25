/*
 * Stock.cpp
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#include "StockManager.h"
#include "data_driver/KDataDriver.h"
#include "utilities/util.h"

namespace hku {

//默认Stock的KData读取驱动，防止没有指定KData驱动
static KDataDriverPtr g_kdataDefaultDriver(new KDataDriver);

const string Stock::default_market;
const string Stock::default_code ;
const string Stock::default_market_code;
const string Stock::default_name;
const hku_uint32 Stock::default_type = Null<hku_uint32>();
const bool Stock::default_valid = false;
const Datetime Stock::default_startDate;// = Null<Datetime>();
const Datetime Stock::default_lastDate;// = Null<Datetime>();
const price_t Stock::default_tick = 0.01;
const price_t Stock::default_tickValue = 0.01;
const price_t Stock::default_unit = 1.0;
const int Stock::default_precision = 2;
const size_t Stock::default_minTradeNumber = 100;
const size_t Stock::default_maxTradeNumber = 1000000;

HKU_API std::ostream& operator <<(std::ostream &os, const Stock& stock) {
    string strip(", ");
    StockManager& sm = StockManager::instance();
    StockTypeInfo typeInfo(sm.getStockTypeInfo(stock.type()));
    os << "Stock(" << stock.market() << strip << stock.code() << strip
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX >= 0x03000000)
       << utf8_to_gb(stock.name()) << strip
       << utf8_to_gb(typeInfo.description()) << strip
#else
       << stock.name() << strip
       << typeInfo.description() << strip
#endif
       << stock.valid() << strip << stock.startDatetime() << strip
       << stock.lastDatetime() << ")";
    return os;
}

string Stock::toString() const {
    std::stringstream os;
    string strip(", ");
    StockManager& sm = StockManager::instance();
    StockTypeInfo typeInfo(sm.getStockTypeInfo(type()));
    os << "Stock(" << market() << strip << code() << strip
       << name() << strip
       << typeInfo.description() << strip
       << valid() << strip << startDatetime() << strip
       << lastDatetime() << ")";
    return os.str();
}


Stock::Data::Data()
: m_market(default_market),
  m_code(default_code),
  m_name(default_name),
  m_type(default_type),
  m_valid(default_valid),
  m_startDate(default_startDate),
  m_lastDate(default_lastDate),
  m_tick(default_tick),
  m_tickValue(default_tickValue),
  m_unit(default_unit),
  m_precision(default_precision),
  m_minTradeNumber(default_minTradeNumber),
  m_maxTradeNumber(default_maxTradeNumber) {
    for (int i = 0; i < KQuery::INVALID_KTYPE; ++i) {
        pKData[i] = KRecordListPtr();
    }
}


Stock::Data::Data(const string& market, const string& code,
      const string& name, hku_uint32 type, bool valid,
      const Datetime& startDate, const Datetime& lastDate,
      price_t tick, price_t tickValue, int precision,
      size_t minTradeNumber, size_t maxTradeNumber)
: m_market(market),
  m_code(code),
  m_name(name),
  m_type(type),
  m_valid(valid),
  m_startDate(startDate),
  m_lastDate(lastDate),
  m_tick(tick),
  m_tickValue(tickValue),
  m_precision(precision),
  m_minTradeNumber(minTradeNumber),
  m_maxTradeNumber(maxTradeNumber) {
    if (0.0 == m_tick) {
        HKU_WARN("tick should not be zero! [Stock::Data::Data]");
        m_unit = 1.0;
    } else {
        m_unit = m_tickValue / m_tick;
    }

    boost::to_upper(m_market);
    m_market_code = m_market + m_code;
    for (int i = 0; i < KQuery::INVALID_KTYPE; ++i) {
        pKData[i] = KRecordListPtr();
    }
}

Stock::Data::~Data() {
    /*for (int i = 0; i < KQuery::INVALID_KTYPE; ++i) {
        delete pKData[i];
    }*/
}


Stock::Stock() {
    //m_data = shared_ptr<Data>(new Data);
    m_kdataDriver = g_kdataDefaultDriver;
}


Stock::~Stock() {

}


Stock::Stock(const Stock& x) {
    if (m_data != x.m_data)
        m_data = x.m_data;

    if (m_kdataDriver != x.m_kdataDriver)
        m_kdataDriver = x.m_kdataDriver;
}


Stock& Stock::operator=(const Stock& x) {
    if(this == &x)
        return *this;

    if (m_data != x.m_data)
        m_data = x.m_data;

    if (m_kdataDriver != x.m_kdataDriver)
        m_kdataDriver = x.m_kdataDriver;
    return *this;
}


Stock::Stock(const string& market,
        const string& code, const string& name) {
    m_data = shared_ptr<Data>(
            new Data(market, code, name,
                    default_type, default_valid,
                    default_startDate, default_lastDate,
                    default_tick, default_tickValue, default_precision,
                    default_minTradeNumber, default_maxTradeNumber));
    m_kdataDriver = g_kdataDefaultDriver;
}


Stock::Stock(const string& market, const string& code,
      const string& name, hku_uint32 type, bool valid,
      const Datetime& startDate, const Datetime& lastDate) {
    m_data = shared_ptr<Data>(
            new Data(market, code, name, type, valid,
                     startDate, lastDate,
                     default_tick, default_tickValue, default_precision,
                     default_minTradeNumber, default_maxTradeNumber));
    m_kdataDriver = g_kdataDefaultDriver;
}


Stock::Stock(const string& market, const string& code,
      const string& name, hku_uint32 type, bool valid,
      const Datetime& startDate, const Datetime& lastDate,
      price_t tick, price_t tickValue, int precision,
      size_t minTradeNumber, size_t maxTradeNumber) {
    m_data = shared_ptr<Data>(
            new Data(market, code, name, type, valid,
                    startDate, lastDate, tick, tickValue, precision,
                    minTradeNumber, maxTradeNumber));
    m_kdataDriver = g_kdataDefaultDriver;
}


bool Stock::operator!=(const Stock& stock) const {
    if (this == &stock)
        return false;

    if (m_data == stock.m_data)
        return false;

    if (!m_data || !stock.m_data)
        return true;

    if( m_data->m_code != stock.code() || m_data->m_market != stock.market())
        return true;

    return false;
}

const string& Stock::market() const {
    return m_data ? m_data->m_market : default_market;
}

const string& Stock::code() const {
    return m_data ? m_data->m_code : default_code;
}

const string& Stock::market_code() const {
    return m_data ? m_data->m_market_code : default_market_code;
}

const string& Stock::name() const {
    return m_data ? m_data->m_name : default_name;
}

hku_uint32 Stock::type() const {
    return m_data ? m_data->m_type : default_type;
}

bool Stock::valid() const {
    return m_data ? m_data->m_valid : default_valid;
}

Datetime Stock::startDatetime() const {
    return m_data ? m_data->m_startDate : default_startDate;
}

Datetime Stock::lastDatetime() const {
    return m_data ? m_data->m_lastDate : default_lastDate;
}

price_t Stock::tick() const {
    return m_data ? m_data->m_tick : default_tick;
}

price_t Stock::tickValue() const {
    return m_data ? m_data->m_tickValue : default_tickValue;
}

price_t Stock::unit() const {
    return m_data ? m_data->m_unit : default_unit;
}

int Stock::precision() const {
    return m_data ? m_data->m_precision : default_precision;
}

size_t Stock::atom() const {
    return m_data ? m_data->m_minTradeNumber : default_minTradeNumber;
}

size_t Stock::minTradeNumber() const {
    return m_data ? m_data->m_minTradeNumber : default_minTradeNumber;
}

size_t Stock::maxTradeNumber() const {
    return m_data ? m_data->m_maxTradeNumber : default_maxTradeNumber;
}

void Stock::setKDataDriver(const KDataDriverPtr& kdataDriver) {
    if (!kdataDriver) {//防止被置为空指针
        m_kdataDriver = g_kdataDefaultDriver;
    }
    m_kdataDriver = kdataDriver;

    for (int i = 0; i < KQuery::INVALID_KTYPE; ++i) {
        releaseKDataBuffer((KQuery::KType)i);
    }
}

void Stock::setWeightList(const StockWeightList& weightList) {
    if (m_data)
        m_data->m_weightList = weightList;
}


KDataDriverPtr Stock::getKDataDriver() const {
    return m_kdataDriver;
}


bool Stock::isBuffer(KQuery::KType ktype) const {
    if (!m_data)
        return false;
    return m_data->pKData[ktype] ? true : false;
}

bool Stock::isNull() const {
    if (!m_data || m_kdataDriver == g_kdataDefaultDriver) {
        return true;
    }
    return false;
}


void Stock::releaseKDataBuffer(KQuery::KType kType) {
    if (!m_data || kType >= KQuery::INVALID_KTYPE)
        return;

    if (m_data->pKData[kType])
        m_data->pKData[kType] = KRecordListPtr();
    return;
}


void Stock::loadKDataToBuffer(KQuery::KType kType) {
    if (!m_data || kType >= KQuery::INVALID_KTYPE)
        return;

    releaseKDataBuffer(kType);
    m_data->pKData[kType] = make_shared<KRecordList>();
    m_kdataDriver->loadKData(m_data->m_market, m_data->m_code,
            kType, 0, Null<size_t>(), m_data->pKData[kType]);
    return;
}


StockWeightList Stock::getWeight(const Datetime& start,
                                 const Datetime& end) const {
    StockWeightList result;
    if (!m_data || start >= end)
        return result;

    StockWeightList::const_iterator start_iter, end_iter;
    start_iter = lower_bound(m_data->m_weightList.begin(),
                             m_data->m_weightList.end(),
                             start, std::less<StockWeight>());
    if( start_iter == m_data->m_weightList.end() ) {
        return result;
    }

    end_iter = lower_bound(start_iter,
                  (StockWeightList::const_iterator)m_data->m_weightList.end(),
                   end, std::less<StockWeight>());
    for(; start_iter != end_iter; start_iter++) {
        result.push_back(*start_iter);
    }

    return result;
}

KData Stock::getKData(const KQuery& query) const {
    return KData(*this, query);
}


size_t Stock::getCount(KQuery::KType kType) const {
    if (!m_data || kType >= KQuery::INVALID_KTYPE)
        return 0;

    if (m_data->pKData[kType])
        return m_data->pKData[kType]->size();

    return m_kdataDriver->getCount(market(), code(), kType);
}


price_t Stock
::getMarketValue(const Datetime& datetime, KQuery::KType ktype) const{
    if (isNull()) {
        return 0.0;
    }

    if (!valid()) {
        if (datetime > lastDatetime()) {
            return 0.0;
        }
    }

    KQuery query = KQueryByDate(datetime, Null<Datetime>(), ktype);
    price_t price = 0.0;
    size_t out_start, out_end;
    KRecord k;
    if (getIndexRange(query, out_start, out_end)) {
        //找到的是>=datetime的记录
        k = getKRecord(out_start, ktype);
        if (k.datetime == datetime) {
            price = k.closePrice;
        } else {
            if (out_start != 0) {
                k = getKRecord(out_start - 1, ktype);
                price = k.closePrice;
            }
        }

    } else {
        //没有找到，则取最后一条记录
        size_t total = getCount(ktype);
        if (total > 0) {
            price = getKRecord(total - 1, ktype).closePrice;
        }
    }

    return price;
}


bool
Stock::getIndexRange(const KQuery& query, size_t& out_start, size_t& out_end)  const{
    out_start = 0;
    out_end = 0;
    if (!m_data || !m_kdataDriver)
        return false;

    if (KQuery::INDEX == query.queryType())
        return _getIndexRangeByIndex(query, out_start, out_end);

    if ((KQuery::DATE != query.queryType())
            || (query.kType() >= KQuery::INVALID_KTYPE)
            || query.startDatetime() >= query.endDatetime())
        return false;

    if (m_data->pKData[query.kType()]) {
        return _getIndexRangeByDateFromBuffer(query, out_start, out_end);
    }

    if (!m_kdataDriver->getIndexRangeByDate(m_data->m_market, m_data->m_code,
            query, out_start, out_end)) {
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

bool Stock::
_getIndexRangeByIndex(const KQuery& query, size_t& out_start, size_t& out_end) const {
    assert(query.queryType() == KQuery::INDEX);
    out_start = 0;
    out_end = 0;

    size_t total = getCount(query.kType());
    if (0 == total) {
        return false;
    }

    hku_int64 startix, endix;
    startix = query.start();
    if(startix < 0) {
        startix += total;
        if(startix < 0)
            startix = 0;
    }

    endix = query.end();
    if(endix < 0) {
        endix += total;
        if(endix < 0)
            endix = 0;
    }

    size_t null_size_t = Null<size_t>();
    size_t startpos = 0;
    size_t endpos = null_size_t;

    try {
        startpos = boost::numeric_cast<size_t>(startix);
    } catch(...) {
        startpos = null_size_t;
    }

    try {
        endpos = boost::numeric_cast<size_t>(endix);
    } catch (...) {
        endpos = null_size_t;
    }

    if(endpos > total) {
        endpos = total;
    }

    if(startpos >= endpos) {
        return false;
    }

    out_start = startpos;
    out_end = endpos;
    return true;
}


bool Stock::
_getIndexRangeByDateFromBuffer(const KQuery& query,
        size_t& out_start, size_t& out_end) const {
    out_start = 0;
    out_end = 0;

    //总数为0，视为失败
    size_t total = getCount(query.kType());
    if( 0 == total ){
        return false;
    }

    KRecordList& kdata = *(m_data->pKData[query.kType()]);
    size_t mid, low = 0, high = total - 1;
    size_t startpos, endpos;
    while(low <= high) {
        if(query.startDatetime() > kdata[high].datetime) {
            mid = high + 1;
            break;
        }

        if(kdata[low].datetime >= query.startDatetime()) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if(query.startDatetime() > kdata[mid].datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if(mid >= total) {
        return false;
    }

    startpos = mid;

    low = mid;
    high = total - 1;
    while(low <= high) {
        if(query.endDatetime() > kdata[high].datetime) {
            mid = high + 1;
            break;
        }

        if(kdata[low].datetime >= query.endDatetime()) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if(query.endDatetime() > kdata[mid].datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    endpos = (mid >= total) ? total : mid;
    if(startpos >= endpos) {
        return false;
    }

    out_start = startpos;
    out_end = endpos;

    return true;
}


KRecord Stock
::getKRecord(size_t pos, KQuery::KType kType) const {
    //if (!m_data)
    //    return KRecord();

    if (m_data->pKData[kType])
        return m_data->pKData[kType]->at(pos);
    return m_kdataDriver->getKRecord(market(), code(), pos, kType);
}


KRecordList Stock
::getKRecordList(size_t start_ix, size_t end_ix, KQuery::KType ktype) const {
    KRecordList result;
    if (!m_data)
        return result;

    if (m_data->pKData[ktype]) {
        size_t total = m_data->pKData[ktype]->size();
        if (start_ix >= end_ix || start_ix > total) {
            HKU_WARN("Invalid param! (" << start_ix << ", "
                    << end_ix << ") [Stock::getKRecordList]");
            return result;
        }

        size_t end = end_ix > total ? total : end_ix;
        result.reserve(end - start_ix);
        for (size_t i = start_ix; i < end; ++i) {
            result.push_back((*m_data->pKData[ktype])[i]);
        }
        return result;
    }

    KRecordListPtr plist(new KRecordList);
    m_kdataDriver->loadKData(m_data->m_market, m_data->m_code,
            ktype, start_ix, end_ix, plist);
    size_t total = plist->size();
    result.reserve(total);
    for (size_t i = 0; i < total; i++) {
        result.push_back((*plist)[i]);
    }
    return result;
}


DatetimeList Stock
::getDatetimeList(size_t start, size_t end, KQuery::KType ktype) const {
    DatetimeList result;
    KRecordList kdata = getKRecordList(start, end, ktype);
    size_t total = kdata.size();
    if (0 == total) {
        return result;
    }

    result.reserve(total);
    for (size_t i = 0; i < kdata.size(); ++i) {
        result.push_back(kdata[i].datetime);
    }
    return result;
}

DatetimeList Stock::getDatetimeList(const KQuery& query) const {
    DatetimeList result;
    size_t start = 0, end = 0;
    if (getIndexRange(query, start, end)) {
        result = getDatetimeList(start, end, query.kType());
    }
    return result;
}


KRecord Stock::
getKRecordByDate(const Datetime& datetime, KQuery::KType ktype) const {
    size_t startix = 0, endix = 0;
    KQuery query = KQueryByDate(datetime, Datetime(datetime.number() + 1), ktype);
    if (getIndexRange(query, startix, endix)) {
        return getKRecord(startix, ktype);
    }

    return Null<KRecord>();
}


void Stock::realtimeUpdate(const KRecord& record) {
    if (!m_data || !m_data->pKData[KQuery::DAY] ||
            record.datetime == Null<Datetime>()) {
        return;
    }

    KRecord &tmp = m_data->pKData[KQuery::DAY]->back();
    if (tmp.datetime == record.datetime) {
        tmp = record;
    } else {
        m_data->pKData[KQuery::DAY]->push_back(record);
    }
}

} /* namespace */
