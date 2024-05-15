/*
 * Stock.cpp
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#include "GlobalInitializer.h"
#include "StockManager.h"
#include "data_driver/KDataDriver.h"
#include "KData.h"

namespace hku {

const string Stock::default_market;
const string Stock::default_code;
const string Stock::default_market_code;
const string Stock::default_name;
const uint32_t Stock::default_type = Null<uint32_t>();
const bool Stock::default_valid = false;
const Datetime Stock::default_startDate;  // = Null<Datetime>();
const Datetime Stock::default_lastDate;   // = Null<Datetime>();
const price_t Stock::default_tick = 0.01;
const price_t Stock::default_tickValue = 0.01;
const price_t Stock::default_unit = 1.0;
const int Stock::default_precision = 2;
const size_t Stock::default_minTradeNumber = 100;
const size_t Stock::default_maxTradeNumber = 1000000;

HKU_API std::ostream& operator<<(std::ostream& os, const Stock& stock) {
    string strip(", ");
    const StockManager& sm = StockManager::instance();
    StockTypeInfo typeInfo(sm.getStockTypeInfo(stock.type()));
    os << "Stock(" << stock.market() << strip << stock.code() << strip << stock.name() << strip
       << typeInfo.description() << strip << stock.valid() << strip << stock.startDatetime()
       << strip << stock.lastDatetime() << ")";
    return os;
}

string Stock::toString() const {
    std::stringstream os;
    string strip(", ");
    const StockManager& sm = StockManager::instance();
    StockTypeInfo typeInfo(sm.getStockTypeInfo(type()));
    os << "Stock(" << market() << strip << code() << strip << name() << strip
       << typeInfo.description() << strip << valid() << strip << startDatetime() << strip
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
    const auto& ktype_list = KQuery::getAllKType();
    for (const auto& ktype : ktype_list) {
        pKData[ktype] = nullptr;
        pMutex[ktype] = nullptr;
    }
}

Stock::Data::Data(const string& market, const string& code, const string& name, uint32_t type,
                  bool valid, const Datetime& startDate, const Datetime& lastDate, price_t tick,
                  price_t tickValue, int precision, double minTradeNumber, double maxTradeNumber)
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
        HKU_WARN("tick should not be zero! now use as 1.0");
        m_unit = 1.0;
    } else {
        m_unit = m_tickValue / m_tick;
    }

    to_upper(m_market);
    m_market_code = marketCode();

    const auto& ktype_list = KQuery::getAllKType();
    for (const auto& ktype : ktype_list) {
        pMutex[ktype] = new std::shared_mutex();
        pKData[ktype] = nullptr;
    }
}

string Stock::Data::marketCode() const {
    if (m_type == STOCKTYPE_CRYPTO)
        return m_market + "/" + m_code;
    return m_market + m_code;
}

Stock::Data::~Data() {
    for (auto iter = pKData.begin(); iter != pKData.end(); ++iter) {
        if (iter->second) {
            delete iter->second;
        }
    }

    for (auto iter = pMutex.begin(); iter != pMutex.end(); ++iter) {
        if (iter->second) {
            delete iter->second;
        }
    }
}

Stock::Stock() {}

Stock::~Stock() {}

Stock::Stock(const Stock& x) : m_data(x.m_data), m_kdataDriver(x.m_kdataDriver) {}

Stock::Stock(Stock&& x) : m_data(std::move(x.m_data)), m_kdataDriver(std::move(x.m_kdataDriver)) {}

Stock& Stock::operator=(const Stock& x) {
    HKU_IF_RETURN(this == &x, *this);
    m_data = x.m_data;
    m_kdataDriver = x.m_kdataDriver;
    return *this;
}

Stock& Stock::operator=(Stock&& x) {
    HKU_IF_RETURN(this == &x, *this);
    m_data = std::move(x.m_data);
    m_kdataDriver = std::move(x.m_kdataDriver);
    return *this;
}

Stock::Stock(const string& market, const string& code, const string& name) {
    m_data = make_shared<Data>(market, code, name, default_type, default_valid, default_startDate,
                               default_lastDate, default_tick, default_tickValue, default_precision,
                               default_minTradeNumber, default_maxTradeNumber);
}

Stock::Stock(const string& market, const string& code, const string& name, uint32_t type,
             bool valid, const Datetime& startDate, const Datetime& lastDate) {
    m_data = make_shared<Data>(market, code, name, type, valid, startDate, lastDate, default_tick,
                               default_tickValue, default_precision, default_minTradeNumber,
                               default_maxTradeNumber);
}

Stock::Stock(const string& market, const string& code, const string& name, uint32_t type,
             bool valid, const Datetime& startDate, const Datetime& lastDate, price_t tick,
             price_t tickValue, int precision, size_t minTradeNumber, size_t maxTradeNumber)
: m_data(make_shared<Data>(market, code, name, type, valid, startDate, lastDate, tick, tickValue,
                           precision, minTradeNumber, maxTradeNumber)) {}

bool Stock::operator==(const Stock& stock) const {
    return this == &stock || m_data == stock.m_data ||
           (m_data && stock.m_data && (m_data->m_code == stock.m_data->m_code) &&
            (m_data->m_market == stock.m_data->m_market));
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

uint32_t Stock::type() const {
    return m_data ? m_data->m_type : default_type;
}

bool Stock::valid() const {
    return m_data ? m_data->m_valid : default_valid;
}

const Datetime& Stock::startDatetime() const {
    return m_data ? m_data->m_startDate : default_startDate;
}

const Datetime& Stock::lastDatetime() const {
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

double Stock::atom() const {
    return m_data ? m_data->m_minTradeNumber : default_minTradeNumber;
}

double Stock::minTradeNumber() const {
    return m_data ? m_data->m_minTradeNumber : default_minTradeNumber;
}

double Stock::maxTradeNumber() const {
    return m_data ? m_data->m_maxTradeNumber : default_maxTradeNumber;
}

void Stock::market(const string& market_) {
    string n_market(market_);
    to_upper(n_market);
    if (!m_data) {
        m_data =
          make_shared<Data>(n_market, default_code, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_market = n_market;
        m_data->m_market_code = m_data->marketCode();
    }
}

void Stock::code(const string& code_) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, code_, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_code = code_;
        m_data->m_market_code = m_data->marketCode();
    }
}

void Stock::name(const string& name_) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, name_, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_name = name_;
    }
}

void Stock::type(uint32_t type_) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, type_, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_type = type_;
    }
}

void Stock::valid(bool valid_) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, valid_,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_valid = valid_;
    }
}

void Stock::precision(int precision_) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            precision_, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_precision = precision_;
    }
}

void Stock::startDatetime(const Datetime& date) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            date, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_startDate = date;
    }
}

void Stock::lastDatetime(const Datetime& date) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            default_startDate, date, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    } else {
        m_data->m_lastDate = date;
    }
}

void Stock::tick(price_t tick_) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    }
    m_data->m_tick = tick_;
    if (0.0 == m_data->m_tick) {
        HKU_WARN("tick should not be zero! now use as 1.0");
        m_data->m_unit = 1.0;
    } else {
        m_data->m_unit = m_data->m_tickValue / m_data->m_tick;
    }
}

void Stock::tickValue(price_t val) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, default_maxTradeNumber);
    }
    m_data->m_tickValue = val;
    if (0.0 == m_data->m_tick) {
        HKU_WARN("tick should not be zero! now use as 1.0");
        m_data->m_unit = 1.0;
    } else {
        m_data->m_unit = m_data->m_tickValue / m_data->m_tick;
    }
}

void Stock::minTradeNumber(double num) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, num, default_maxTradeNumber);
    } else {
        m_data->m_minTradeNumber = num;
    }
}

void Stock::maxTradeNumber(double num) {
    if (!m_data) {
        m_data =
          make_shared<Data>(default_market, default_code, default_name, default_type, default_valid,
                            default_startDate, default_lastDate, default_tick, default_tickValue,
                            default_precision, default_minTradeNumber, num);
    } else {
        m_data->m_maxTradeNumber = num;
    }
}

void Stock::setKDataDriver(const KDataDriverConnectPoolPtr& kdataDriver) {
    HKU_CHECK(kdataDriver, "kdataDriver is nullptr!");
    m_kdataDriver = kdataDriver;
    if (m_data) {
        auto& ktype_list = KQuery::getAllKType();
        for (auto& ktype : ktype_list) {
            std::unique_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
            delete m_data->pKData[ktype];
            m_data->pKData[ktype] = nullptr;
        }
    }
}

KDataDriverConnectPoolPtr Stock::getKDataDirver() const {
    return m_kdataDriver;
}

void Stock::setWeightList(const StockWeightList& weightList) {
    if (m_data) {
        std::lock_guard<std::mutex> lock(m_data->m_weight_mutex);
        m_data->m_weightList = weightList;
    }
}

bool Stock::isBuffer(KQuery::KType ktype) const {
    HKU_IF_RETURN(!m_data, false);
    string nktype(ktype);
    to_upper(nktype);
    return m_data->pKData.find(nktype) != m_data->pKData.end() && m_data->pKData[nktype];
}

bool Stock::isNull() const {
    return !m_data || !m_kdataDriver;
}

void Stock::releaseKDataBuffer(KQuery::KType inkType) {
    HKU_IF_RETURN(!m_data, void());

    string ktype(inkType);
    to_upper(ktype);
    HKU_IF_RETURN(m_data->pMutex.find(ktype) == m_data->pMutex.end(), void());

    std::unique_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
    auto iter = m_data->pKData.find(ktype);
    if (iter->second) {
        delete iter->second;
        iter->second = nullptr;
    }
}

// 仅在初始化时调用
void Stock::loadKDataToBuffer(KQuery::KType inkType) {
    HKU_IF_RETURN(!m_data || !m_kdataDriver, void());

    string kType(inkType);
    to_upper(kType);
    HKU_IF_RETURN(m_data->pKData.find(kType) == m_data->pKData.end(), void());

    releaseKDataBuffer(kType);

    int start = 0;
    auto driver = m_kdataDriver->getConnect();
    size_t total = driver->getCount(m_data->m_market, m_data->m_code, kType);

    // CSV 直接全部加载至内存，其他类型依据配置的预加载参数进行加载
    if (driver->name() != "TMPCSV") {
        const auto& param = StockManager::instance().getPreloadParameter();
        string preload_type = fmt::format("{}_max", kType);
        to_lower(preload_type);
        int max_num = param.tryGet<int>(preload_type, 4096);
        HKU_ERROR_IF_RETURN(max_num < 0, void(), "Invalid preload {} param: {}", preload_type,
                            max_num);
        start = total <= max_num ? 0 : total - max_num;
    }

    {
        std::unique_lock<std::shared_mutex> lock(*(m_data->pMutex[kType]));
        // 需要对是否已缓存进行二次判定，防止加锁之前已被缓存
        if (m_data->pKData.find(kType) != m_data->pKData.end() && m_data->pKData[kType]) {
            return;
        }
        KRecordList* ptr_klist = new KRecordList;
        m_data->pKData[kType] = ptr_klist;
        if (total != 0) {
            (*ptr_klist) = driver->getKRecordList(m_data->m_market, m_data->m_code,
                                                  KQuery(start, Null<int64_t>(), kType));
        }
    }
}

StockWeightList Stock::getWeight(const Datetime& start, const Datetime& end) const {
    StockWeightList result;
    HKU_IF_RETURN(!m_data || start >= end, result);
    std::lock_guard<std::mutex> lock(m_data->m_weight_mutex);
    StockWeightList::const_iterator start_iter, end_iter;
    start_iter = lower_bound(m_data->m_weightList.begin(), m_data->m_weightList.end(),
                             StockWeight(start), std::less<StockWeight>());
    if (start_iter == m_data->m_weightList.end()) {
        return result;
    }

    end_iter = lower_bound(start_iter, (StockWeightList::const_iterator)m_data->m_weightList.end(),
                           StockWeight(end), std::less<StockWeight>());
    for (; start_iter != end_iter; ++start_iter) {
        result.push_back(*start_iter);
    }

    return result;
}

KData Stock::getKData(const KQuery& query) const {
    return KData(*this, query);
}

size_t Stock::_getCountFromBuffer(KQuery::KType ktype) const {
    std::shared_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
    return m_data->pKData[ktype]->size();
}

size_t Stock::getCount(KQuery::KType kType) const {
    HKU_IF_RETURN(!m_data, 0);
    string nktype(kType);
    to_upper(nktype);
    if (m_data->pKData.find(nktype) != m_data->pKData.end() && m_data->pKData[nktype]) {
        return _getCountFromBuffer(nktype);
    }

    return m_kdataDriver ? m_kdataDriver->getConnect()->getCount(market(), code(), nktype) : 0;
}

price_t Stock::getMarketValue(const Datetime& datetime, KQuery::KType inktype) const {
    HKU_IF_RETURN(isNull(), 0.0);
    HKU_IF_RETURN(!valid() && datetime > lastDatetime(), 0.0);

    string ktype(inktype);
    to_upper(ktype);

    // 如果为内存缓存或者数据驱动为索引优先，则按索引方式获取
    if (isBuffer(ktype) || m_kdataDriver->getConnect()->isIndexFirst()) {
        KQuery query = KQueryByDate(datetime, Null<Datetime>(), ktype);
        size_t out_start, out_end;
        if (getIndexRange(query, out_start, out_end)) {
            // 找到的是>=datetime的记录
            KRecord k = getKRecord(out_start, ktype);
            if (k.datetime == datetime) {
                return k.closePrice;
            }
            if (out_start != 0) {
                k = getKRecord(out_start - 1, ktype);
                return k.closePrice;
            }
        }

    } else {
        // 未在缓存中，且日期优先的情况下
        // 先尝试获取等于该日期的K线数据
        // 如未找到，则获取小于该日期的最后一条记录
        KQuery query = KQueryByDate(datetime, datetime + Minutes(1), ktype);
        auto k_list = getKRecordList(query);
        if (k_list.size() > 0 && k_list[0].datetime == datetime) {
            return k_list[0].closePrice;
        }

        query = KQueryByDate(startDatetime(), datetime, ktype);
        k_list = getKRecordList(query);
        if (k_list.size() > 0) {
            return k_list[k_list.size() - 1].closePrice;
        }
    }

    // 没有找到，则取最后一条记录
    price_t price = 0.0;
    size_t total = getCount(ktype);
    if (total > 0) {
        price = getKRecord(total - 1, ktype).closePrice;
    }

    return price;
}

bool Stock::getIndexRange(const KQuery& query, size_t& out_start, size_t& out_end) const {
    out_start = 0;
    out_end = 0;
    HKU_IF_RETURN(!m_data || !m_kdataDriver, false);

    if (KQuery::INDEX == query.queryType())
        return _getIndexRangeByIndex(query, out_start, out_end);

    if ((KQuery::DATE != query.queryType()) || query.startDatetime() >= query.endDatetime())
        return false;

    if (isBuffer(query.kType())) {
        return _getIndexRangeByDateFromBuffer(query, out_start, out_end);
    }

    if (!m_kdataDriver->getConnect()->getIndexRangeByDate(m_data->m_market, m_data->m_code, query,
                                                          out_start, out_end)) {
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

bool Stock::_getIndexRangeByIndex(const KQuery& query, size_t& out_start, size_t& out_end) const {
    assert(query.queryType() == KQuery::INDEX);
    out_start = 0;
    out_end = 0;

    size_t total = getCount(query.kType());
    HKU_IF_RETURN(0 == total, false);

    int64_t startix, endix;
    startix = query.start();
    if (startix < 0) {
        startix += total;
        if (startix < 0)
            startix = 0;
    }

    endix = query.end();
    if (endix < 0) {
        endix += total;
        if (endix < 0)
            endix = 0;
    }

    size_t null_size_t = Null<size_t>();
    size_t startpos = 0;
    size_t endpos = null_size_t;

    try {
        startpos = boost::numeric_cast<size_t>(startix);
    } catch (...) {
        startpos = null_size_t;
    }

    try {
        endpos = boost::numeric_cast<size_t>(endix);
    } catch (...) {
        endpos = null_size_t;
    }

    if (endpos > total) {
        endpos = total;
    }

    if (startpos >= endpos) {
        return false;
    }

    out_start = startpos;
    out_end = endpos;
    return true;
}

bool Stock::_getIndexRangeByDateFromBuffer(const KQuery& query, size_t& out_start,
                                           size_t& out_end) const {
    std::shared_lock<std::shared_mutex> lock(*(m_data->pMutex[query.kType()]));
    out_start = 0;
    out_end = 0;

    const KRecordList& kdata = *(m_data->pKData[query.kType()]);
    size_t total = kdata.size();
    HKU_IF_RETURN(0 == total, false);

    size_t mid = total, low = 0, high = total - 1;
    size_t startpos, endpos;
    while (low <= high) {
        if (query.startDatetime() > kdata[high].datetime) {
            mid = high + 1;
            break;
        }

        if (kdata[low].datetime >= query.startDatetime()) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if (query.startDatetime() > kdata[mid].datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (mid >= total) {
        return false;
    }

    startpos = mid;

    low = mid;
    high = total - 1;
    while (low <= high) {
        if (query.endDatetime() > kdata[high].datetime) {
            mid = high + 1;
            break;
        }

        if (kdata[low].datetime >= query.endDatetime()) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if (query.endDatetime() > kdata[mid].datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    endpos = (mid >= total) ? total : mid;
    if (startpos >= endpos) {
        return false;
    }

    out_start = startpos;
    out_end = endpos;

    return true;
}

KRecord Stock::_getKRecordFromBuffer(size_t pos, const KQuery::KType& ktype) const {
    std::shared_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
    return m_data->pKData[ktype]->at(pos);
}

KRecord Stock::getKRecord(size_t pos, const KQuery::KType& kType) const {
    HKU_IF_RETURN(!m_data, Null<KRecord>());
    if (isBuffer(kType)) {
        return _getKRecordFromBuffer(pos, kType);
    }

    HKU_IF_RETURN(!m_kdataDriver || pos >= size_t(Null<int64_t>()), Null<KRecord>());
    auto klist =
      m_kdataDriver->getConnect()->getKRecordList(market(), code(), KQuery(pos, pos + 1, kType));
    return klist.size() > 0 ? klist[0] : Null<KRecord>();
}

KRecord Stock::getKRecord(const Datetime& datetime, const KQuery::KType& ktype) const {
    KRecord result;
    HKU_IF_RETURN(isNull(), result);

    KQuery query = KQueryByDate(datetime, datetime + Minutes(1), ktype);
    auto driver = m_kdataDriver->getConnect();
    if (isBuffer(query.kType()) || driver->isIndexFirst()) {
        size_t startix = 0, endix = 0;
        return getIndexRange(query, startix, endix) ? getKRecord(startix, ktype) : Null<KRecord>();
    }

    auto klist = driver->getKRecordList(market(), code(), query);
    return klist.size() > 0 ? klist[0] : Null<KRecord>();
}

KRecordList Stock::_getKRecordListFromBuffer(size_t start_ix, size_t end_ix,
                                             KQuery::KType ktype) const {
    std::shared_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
    KRecordList result;
    size_t total = m_data->pKData[ktype]->size();
    HKU_IF_RETURN(total == 0, result);
    HKU_WARN_IF_RETURN(start_ix >= end_ix || start_ix >= total, result,
                       "Invalid param (start_ix: {}, end_ix: {})! current total: {}", start_ix,
                       end_ix, total);
    size_t length = end_ix > total ? total - start_ix : end_ix - start_ix;
    result.resize(length);
    std::memcpy((void*)&(result.front()), &((*m_data->pKData[ktype])[start_ix]),
                sizeof(KRecord) * length);
    return result;
}

KRecordList Stock::getKRecordList(const KQuery& query) const {
    KRecordList result;
    HKU_IF_RETURN(isNull(), result);

    // 如果是在内存缓存中
    if (isBuffer(query.kType())) {
        size_t start_ix = 0, end_ix = 0;
        if (query.queryType() == KQuery::DATE) {
            if (!_getIndexRangeByDateFromBuffer(query, start_ix, end_ix)) {
                return result;
            }
        } else {
            if (query.start() < 0 || query.end() < 0) {
                // 处理负数索引
                if (!getIndexRange(query, start_ix, end_ix)) {
                    return result;
                }
            } else {
                start_ix = query.start();
                end_ix = query.end();
            }
        }
        result = _getKRecordListFromBuffer(start_ix, end_ix, query.kType());

    } else {
        if (query.queryType() == KQuery::DATE) {
            result =
              m_kdataDriver->getConnect()->getKRecordList(m_data->m_market, m_data->m_code, query);
        } else {
            size_t start_ix = 0, end_ix = 0;
            if (query.queryType() == KQuery::INDEX) {
                if (query.start() < 0 || query.end() < 0) {
                    // 处理负数索引
                    if (!getIndexRange(query, start_ix, end_ix)) {
                        return result;
                    }
                } else {
                    start_ix = query.start();
                    end_ix = query.end();
                }
            }
            result = m_kdataDriver->getConnect()->getKRecordList(
              m_data->m_market, m_data->m_code, KQuery(start_ix, end_ix, query.kType()));
        }
    }

    return result;
}

DatetimeList Stock::getDatetimeList(const KQuery& query) const {
    DatetimeList result;
    KRecordList k_list = getKRecordList(query);
    result.reserve(k_list.size());
    for (auto& k : k_list) {
        result.push_back(k.datetime);  // cppcheck-suppress useStlAlgorithm
    }
    return result;
}

TimeLineList Stock::getTimeLineList(const KQuery& query) const {
    return m_kdataDriver ? m_kdataDriver->getConnect()->getTimeLineList(market(), code(), query)
                         : TimeLineList();
}

TransList Stock::getTransList(const KQuery& query) const {
    return m_kdataDriver ? m_kdataDriver->getConnect()->getTransList(market(), code(), query)
                         : TransList();
}

Parameter Stock::getFinanceInfo() const {
    Parameter result;
    HKU_IF_RETURN(type() != STOCKTYPE_A && type() != STOCKTYPE_GEM && type() != STOCKTYPE_START &&
                    type() != STOCKTYPE_A_BJ,
                  result);

    BaseInfoDriverPtr driver = StockManager::instance().getBaseInfoDriver();
    if (driver) {
        result = driver->getFinanceInfo(market(), code());
    }

    return result;
}

vector<Block> Stock::getBelongToBlockList(const string& category) const {
    return StockManager::instance().getStockBelongs(*this, category);
}

// 判断是否在交易时间段内（不判断日期）
bool Stock::isTransactionTime(Datetime time) {
    MarketInfo market_info = StockManager::instance().getMarketInfo(market());
    HKU_IF_RETURN(market_info == Null<MarketInfo>(), false);

    HKU_ERROR_IF_RETURN(market_info.openTime1() > market_info.closeTime1() ||
                          market_info.openTime2() > market_info.closeTime2(),
                        false, "Error transaction time in market({})!", market_info.market());

    // 如果交易时间段的起始和终止时间都相等则认未做限定，全天都为可交易时间
    HKU_IF_RETURN(market_info.openTime1() == market_info.closeTime1() &&
                    market_info.openTime2() == market_info.closeTime2(),
                  true);

    Datetime today = Datetime::today();
    Datetime openTime1 = today + market_info.openTime1();
    Datetime closeTime1 = today + market_info.closeTime1();
    HKU_IF_RETURN(time >= openTime1 && time < closeTime1, true);  // close判断不包括等于

    Datetime openTime2 = today + market_info.openTime2();
    Datetime closeTime2 = today + market_info.closeTime2();
    return time >= openTime2 && time < closeTime2;
}

void Stock::realtimeUpdate(KRecord record, KQuery::KType inktype) {
    HKU_IF_RETURN(!isBuffer(inktype) || record.datetime.isNull() ||
                    StockManager::instance().isHoliday(record.datetime),
                  void());

    string ktype(inktype);
    to_upper(ktype);

    // 加写锁
    std::unique_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));

    // 需要对是否已缓存进行二次判定，防止加锁之前缓存被释放
    if (m_data->pKData.find(ktype) == m_data->pKData.end() || !m_data->pKData[ktype]) {
        return;
    }

    if (m_data->pKData[ktype]->empty()) {
        m_data->pKData[ktype]->push_back(record);
        return;
    }

    KRecord& tmp = m_data->pKData[ktype]->back();

    // 如果传入的记录日期等于最后一条记录日期，则更新最后一条记录；否则，追加入缓存
    if (tmp.datetime == record.datetime) {
        tmp = record;
    } else if (tmp.datetime < record.datetime) {
        m_data->pKData[ktype]->push_back(record);
    } else {
        HKU_INFO("Ignore record, datetime < last record.datetime!");
    }
}

void Stock::setKRecordList(const KRecordList& ks, const KQuery::KType& ktype) {
    HKU_IF_RETURN(ks.empty(), void());
    string nktype(ktype);
    to_upper(nktype);

    // 写锁
    std::unique_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
    HKU_CHECK(m_data->pKData.find(nktype) != m_data->pKData.end(), "Invalid ktype: {}", ktype);

    if (!m_data->pKData[nktype]) {
        m_data->pKData[nktype] = new KRecordList();
    }

    (*(m_data->pKData[nktype])) = ks;

    Parameter param;
    param.set<string>("type", "DoNothing");
    m_kdataDriver = DataDriverFactory::getKDataDriverPool(param);

    m_data->m_valid = true;
    m_data->m_startDate = ks.front().datetime;
    m_data->m_lastDate = ks.back().datetime;
}

void Stock::setKRecordList(KRecordList&& ks, const KQuery::KType& ktype) {
    HKU_IF_RETURN(ks.empty(), void());
    string nktype(ktype);
    to_upper(nktype);

    // 写锁
    std::unique_lock<std::shared_mutex> lock(*(m_data->pMutex[ktype]));
    HKU_CHECK(m_data->pKData.find(nktype) != m_data->pKData.end(), "Invalid ktype: {}", ktype);

    if (!m_data->pKData[nktype]) {
        m_data->pKData[nktype] = new KRecordList();
    }

    (*m_data->pKData[nktype]) = std::move(ks);

    Parameter param;
    param.set<string>("type", "DoNothing");
    m_kdataDriver = DataDriverFactory::getKDataDriverPool(param);

    m_data->m_valid = true;
    m_data->m_startDate = (*m_data->pKData[nktype]).front().datetime;
    m_data->m_lastDate = (*m_data->pKData[nktype]).back().datetime;
}

const vector<HistoryFinanceInfo>& Stock::getHistoryFinance() const {
    std::lock_guard<std::mutex> lock(m_data->m_history_finance_mutex);
    if (!m_data->m_history_finance_ready) {
        m_data->m_history_finance =
          StockManager::instance().getHistoryFinance(*this, Datetime::min(), Null<Datetime>());
        m_data->m_history_finance_ready = true;
    }
    return m_data->m_history_finance;
}

Stock HKU_API getStock(const string& querystr) {
    const StockManager& sm = StockManager::instance();
    return sm.getStock(querystr);
}

}  // namespace hku
