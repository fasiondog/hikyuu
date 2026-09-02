/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <cstring>
#include "IpcProtocol.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/utilities/Log.h"

namespace hku {
namespace ipc {

///////////////////////////////////////////////////////////////////////////////
// Encoder
///////////////////////////////////////////////////////////////////////////////
void Encoder::putU8(uint8_t v) {
    m_buf.push_back(v);
}

void Encoder::putU16(uint16_t v) {
    size_t pos = m_buf.size();
    m_buf.resize(pos + 2);
    std::memcpy(m_buf.data() + pos, &v, 2);
}

void Encoder::putU32(uint32_t v) {
    size_t pos = m_buf.size();
    m_buf.resize(pos + 4);
    std::memcpy(m_buf.data() + pos, &v, 4);
}

void Encoder::putU64(uint64_t v) {
    size_t pos = m_buf.size();
    m_buf.resize(pos + 8);
    std::memcpy(m_buf.data() + pos, &v, 8);
}

void Encoder::putI32(int32_t v) {
    putU32(static_cast<uint32_t>(v));
}

void Encoder::putI64(int64_t v) {
    putU64(static_cast<uint64_t>(v));
}

void Encoder::putDouble(double v) {
    size_t pos = m_buf.size();
    m_buf.resize(pos + 8);
    std::memcpy(m_buf.data() + pos, &v, 8);
}

void Encoder::putFloat(float v) {
    size_t pos = m_buf.size();
    m_buf.resize(pos + 4);
    std::memcpy(m_buf.data() + pos, &v, 4);
}

void Encoder::putString(const std::string& s) {
    HKU_WARN_IF_RETURN(s.size() > UINT16_MAX, void(), "String too long to encode! len: {}",
                       s.size());
    putU16(static_cast<uint16_t>(s.size()));
    if (!s.empty()) {
        size_t pos = m_buf.size();
        m_buf.resize(pos + s.size());
        std::memcpy(m_buf.data() + pos, s.data(), s.size());
    }
}

void Encoder::putDatetime(const Datetime& d) {
    putU64(d.number());
}

///////////////////////////////////////////////////////////////////////////////
// Reader
///////////////////////////////////////////////////////////////////////////////
bool Reader::check(size_t n) noexcept {
    if (m_ok && m_pos + n <= m_len) {
        return true;
    }
    m_ok = false;
    return false;
}

uint8_t Reader::getU8() {
    HKU_IF_RETURN(!check(1), 0);
    return m_data[m_pos++];
}

uint16_t Reader::getU16() {
    HKU_IF_RETURN(!check(2), 0);
    uint16_t v;
    std::memcpy(&v, m_data + m_pos, 2);
    m_pos += 2;
    return v;
}

uint32_t Reader::getU32() {
    HKU_IF_RETURN(!check(4), 0);
    uint32_t v;
    std::memcpy(&v, m_data + m_pos, 4);
    m_pos += 4;
    return v;
}

uint64_t Reader::getU64() {
    HKU_IF_RETURN(!check(8), 0);
    uint64_t v;
    std::memcpy(&v, m_data + m_pos, 8);
    m_pos += 8;
    return v;
}

int32_t Reader::getI32() {
    return static_cast<int32_t>(getU32());
}

int64_t Reader::getI64() {
    return static_cast<int64_t>(getU64());
}

double Reader::getDouble() {
    HKU_IF_RETURN(!check(8), 0.0);
    double v;
    std::memcpy(&v, m_data + m_pos, 8);
    m_pos += 8;
    return v;
}

float Reader::getFloat() {
    HKU_IF_RETURN(!check(4), 0.0f);
    float v;
    std::memcpy(&v, m_data + m_pos, 4);
    m_pos += 4;
    return v;
}

std::string Reader::getString() {
    uint16_t len = getU16();
    HKU_IF_RETURN(!m_ok, std::string());
    HKU_IF_RETURN(!check(len), std::string());
    std::string s(reinterpret_cast<const char*>(m_data + m_pos), len);
    m_pos += len;
    return s;
}

Datetime Reader::getDatetime() {
    uint64_t n = getU64();
    HKU_IF_RETURN(!m_ok, Datetime());
    return Datetime(n);
}

uint64_t Reader::getCount(size_t elem_min_size) {
    uint64_t count = getU64();
    HKU_IF_RETURN(!m_ok, 0);
    // 超界时必须置失败，否则调用方会把“坏帧”误读为“成功但集合为空”
    if (elem_min_size > 0 && count > remain() / elem_min_size) {
        m_ok = false;
        return 0;
    }
    return count;
}

uint32_t Reader::getCount32(size_t elem_min_size) {
    uint32_t count = getU32();
    HKU_IF_RETURN(!m_ok, 0);
    if (elem_min_size > 0 && count > remain() / elem_min_size) {
        m_ok = false;
        return 0;
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////
// 帧编解码
///////////////////////////////////////////////////////////////////////////////
std::vector<uint8_t> encodeRequest(Cmd cmd, const std::vector<uint8_t>& body) {
    std::vector<uint8_t> frame(REQUEST_HEADER_SIZE + body.size());
    uint32_t magic = MAGIC;
    uint16_t version = VERSION;
    uint16_t cmdv = static_cast<uint16_t>(cmd);
    uint32_t body_len = static_cast<uint32_t>(body.size());
    std::memcpy(frame.data(), &magic, 4);
    std::memcpy(frame.data() + 4, &version, 2);
    std::memcpy(frame.data() + 6, &cmdv, 2);
    std::memcpy(frame.data() + 8, &body_len, 4);
    if (!body.empty()) {
        std::memcpy(frame.data() + REQUEST_HEADER_SIZE, body.data(), body.size());
    }
    return frame;
}

bool decodeRequest(const std::vector<uint8_t>& frame, Cmd& out_cmd,
                   std::vector<uint8_t>& out_body) {
    HKU_ERROR_IF_RETURN(frame.size() < REQUEST_HEADER_SIZE, false, "Request frame too short!");
    uint32_t magic = 0;
    uint16_t version = 0, cmdv = 0;
    uint32_t body_len = 0;
    std::memcpy(&magic, frame.data(), 4);
    std::memcpy(&version, frame.data() + 4, 2);
    std::memcpy(&cmdv, frame.data() + 6, 2);
    std::memcpy(&body_len, frame.data() + 8, 4);
    HKU_ERROR_IF_RETURN(magic != MAGIC, false, "Invalid request magic: {:#x}", magic);
    HKU_ERROR_IF_RETURN(version != VERSION, false, "Unsupported protocol version: {}", version);
    HKU_ERROR_IF_RETURN(frame.size() < REQUEST_HEADER_SIZE + body_len, false,
                        "Request frame body incomplete!");
    out_cmd = static_cast<Cmd>(cmdv);
    out_body.assign(frame.begin() + REQUEST_HEADER_SIZE,
                    frame.begin() + REQUEST_HEADER_SIZE + body_len);
    return true;
}

std::vector<uint8_t> encodeResponse(RetCode ret, const std::vector<uint8_t>& body) {
    std::vector<uint8_t> frame(RESPONSE_HEADER_SIZE + body.size());
    uint32_t magic = MAGIC;
    int32_t retv = static_cast<int32_t>(ret);
    uint32_t body_len = static_cast<uint32_t>(body.size());
    std::memcpy(frame.data(), &magic, 4);
    std::memcpy(frame.data() + 4, &retv, 4);
    std::memcpy(frame.data() + 8, &body_len, 4);
    if (!body.empty()) {
        std::memcpy(frame.data() + RESPONSE_HEADER_SIZE, body.data(), body.size());
    }
    return frame;
}

bool decodeResponse(const std::vector<uint8_t>& frame, RetCode& out_ret,
                    std::vector<uint8_t>& out_body) {
    HKU_ERROR_IF_RETURN(frame.size() < RESPONSE_HEADER_SIZE, false, "Response frame too short!");
    uint32_t magic = 0;
    int32_t retv = 0;
    uint32_t body_len = 0;
    std::memcpy(&magic, frame.data(), 4);
    std::memcpy(&retv, frame.data() + 4, 4);
    std::memcpy(&body_len, frame.data() + 8, 4);
    HKU_ERROR_IF_RETURN(magic != MAGIC, false, "Invalid response magic: {:#x}", magic);
    HKU_ERROR_IF_RETURN(frame.size() < RESPONSE_HEADER_SIZE + body_len, false,
                        "Response frame body incomplete!");
    out_ret = static_cast<RetCode>(retv);
    out_body.assign(frame.begin() + RESPONSE_HEADER_SIZE,
                    frame.begin() + RESPONSE_HEADER_SIZE + body_len);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// KRecord
///////////////////////////////////////////////////////////////////////////////
void encodeKRecord(Encoder& enc, const KRecord& k) {
    enc.putDatetime(k.datetime);
    enc.putDouble(k.openPrice);
    enc.putDouble(k.highPrice);
    enc.putDouble(k.lowPrice);
    enc.putDouble(k.closePrice);
    enc.putDouble(k.transAmount);
    enc.putDouble(k.transCount);
}

KRecord decodeKRecord(Reader& rd) {
    KRecord k;
    k.datetime = rd.getDatetime();
    k.openPrice = rd.getDouble();
    k.highPrice = rd.getDouble();
    k.lowPrice = rd.getDouble();
    k.closePrice = rd.getDouble();
    k.transAmount = rd.getDouble();
    k.transCount = rd.getDouble();
    return k;
}

void encodeKRecordList(Encoder& enc, const KRecordList& ks) {
    enc.putU64(ks.size());
    for (const auto& k : ks) {
        encodeKRecord(enc, k);
    }
}

KRecordList decodeKRecordList(Reader& rd) {
    KRecordList ks;
    // 单条 KRecord 线上最小 56 字节：datetime(8) + 6 个 double(48)
    uint64_t count = rd.getCount(56);
    HKU_IF_RETURN(!rd.ok(), ks);
    ks.resize(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        ks[i] = decodeKRecord(rd);
    }
    return ks;
}

///////////////////////////////////////////////////////////////////////////////
// TimeLineRecord
///////////////////////////////////////////////////////////////////////////////
void encodeTimeLineRecord(Encoder& enc, const TimeLineRecord& t) {
    enc.putDatetime(t.datetime);
    enc.putDouble(t.price);
    enc.putDouble(t.vol);
}

TimeLineRecord decodeTimeLineRecord(Reader& rd) {
    TimeLineRecord t;
    t.datetime = rd.getDatetime();
    t.price = rd.getDouble();
    t.vol = rd.getDouble();
    return t;
}

void encodeTimeLineList(Encoder& enc, const TimeLineList& ts) {
    enc.putU64(ts.size());
    for (const auto& t : ts) {
        encodeTimeLineRecord(enc, t);
    }
}

TimeLineList decodeTimeLineList(Reader& rd) {
    TimeLineList ts;
    // 单条分时记录线上最小 24 字节：datetime(8) + price(8) + vol(8)
    uint64_t count = rd.getCount(24);
    HKU_IF_RETURN(!rd.ok(), ts);
    ts.resize(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        ts[i] = decodeTimeLineRecord(rd);
    }
    return ts;
}

///////////////////////////////////////////////////////////////////////////////
// TransRecord
///////////////////////////////////////////////////////////////////////////////
void encodeTransRecord(Encoder& enc, const TransRecord& t) {
    enc.putDatetime(t.datetime);
    enc.putDouble(t.price);
    enc.putDouble(t.vol);
    enc.putI32(t.direct);
}

TransRecord decodeTransRecord(Reader& rd) {
    TransRecord t;
    t.datetime = rd.getDatetime();
    t.price = rd.getDouble();
    t.vol = rd.getDouble();
    t.direct = rd.getI32();
    return t;
}

void encodeTransList(Encoder& enc, const TransList& ts) {
    enc.putU64(ts.size());
    for (const auto& t : ts) {
        encodeTransRecord(enc, t);
    }
}

TransList decodeTransList(Reader& rd) {
    TransList ts;
    // 单条分笔记录线上最小 28 字节：datetime(8) + price(8) + vol(8) + direct(4)
    uint64_t count = rd.getCount(28);
    HKU_IF_RETURN(!rd.ok(), ts);
    ts.resize(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        ts[i] = decodeTransRecord(rd);
    }
    return ts;
}

///////////////////////////////////////////////////////////////////////////////
// StockWeight
///////////////////////////////////////////////////////////////////////////////
void encodeStockWeight(Encoder& enc, const StockWeight& w) {
    enc.putDatetime(w.datetime());
    enc.putDouble(w.countAsGift());
    enc.putDouble(w.countForSell());
    enc.putDouble(w.priceForSell());
    enc.putDouble(w.bonus());
    enc.putDouble(w.increasement());
    enc.putDouble(w.totalCount());
    enc.putDouble(w.freeCount());
    enc.putDouble(w.suogu());
}

StockWeight decodeStockWeight(Reader& rd) {
    Datetime datetime = rd.getDatetime();
    price_t countAsGift = rd.getDouble();
    price_t countForSell = rd.getDouble();
    price_t priceForSell = rd.getDouble();
    price_t bonus = rd.getDouble();
    price_t increasement = rd.getDouble();
    price_t totalCount = rd.getDouble();
    price_t freeCount = rd.getDouble();
    price_t suogu = rd.getDouble();
    HKU_IF_RETURN(!rd.ok(), StockWeight());
    return StockWeight(datetime, countAsGift, countForSell, priceForSell, bonus, increasement,
                       totalCount, freeCount, suogu);
}

void encodeStockWeightList(Encoder& enc, const StockWeightList& ws) {
    enc.putU64(ws.size());
    for (const auto& w : ws) {
        encodeStockWeight(enc, w);
    }
}

StockWeightList decodeStockWeightList(Reader& rd) {
    StockWeightList ws;
    // 单条权息线上最小 72 字节：datetime(8) + 8 个 double(64)
    uint64_t count = rd.getCount(72);
    HKU_IF_RETURN(!rd.ok(), ws);
    ws.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        ws.emplace_back(decodeStockWeight(rd));
    }
    return ws;
}

///////////////////////////////////////////////////////////////////////////////
// StockInfo
///////////////////////////////////////////////////////////////////////////////
void encodeStockInfo(Encoder& enc, const StockInfo& info) {
    enc.putString(info.market);
    enc.putString(info.code);
    enc.putString(info.name);
    enc.putU32(info.type);
    enc.putU32(info.valid);
    enc.putU64(info.startDate);
    enc.putU64(info.endDate);
    enc.putU32(static_cast<uint32_t>(info.precision));
    enc.putDouble(info.tick);
    enc.putDouble(info.tickValue);
    enc.putDouble(info.minTradeNumber);
    enc.putDouble(info.maxTradeNumber);
}

StockInfo decodeStockInfo(Reader& rd) {
    StockInfo info;
    info.market = rd.getString();
    info.code = rd.getString();
    info.name = rd.getString();
    info.type = rd.getU32();
    info.valid = rd.getU32();
    info.startDate = rd.getU64();
    info.endDate = rd.getU64();
    info.precision = static_cast<int>(rd.getU32());
    info.tick = rd.getDouble();
    info.tickValue = rd.getDouble();
    info.minTradeNumber = rd.getDouble();
    info.maxTradeNumber = rd.getDouble();
    return info;
}

void encodeStockInfoList(Encoder& enc, const std::vector<StockInfo>& infos) {
    enc.putU64(infos.size());
    for (const auto& info : infos) {
        encodeStockInfo(enc, info);
    }
}

std::vector<StockInfo> decodeStockInfoList(Reader& rd) {
    std::vector<StockInfo> infos;
    // 单条证券信息线上最小 66 字节：3 个字符串前缀(6) + 定长字段(60)
    uint64_t count = rd.getCount(66);
    HKU_IF_RETURN(!rd.ok(), infos);
    infos.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        infos.emplace_back(decodeStockInfo(rd));
    }
    return infos;
}

///////////////////////////////////////////////////////////////////////////////
// MarketInfo
///////////////////////////////////////////////////////////////////////////////
void encodeMarketInfo(Encoder& enc, const MarketInfo& info) {
    enc.putString(info.market());
    enc.putString(info.name());
    enc.putString(info.description());
    enc.putString(info.code());
    enc.putDatetime(info.lastDate());
    enc.putI64(info.openTime1().ticks());
    enc.putI64(info.closeTime1().ticks());
    enc.putI64(info.openTime2().ticks());
    enc.putI64(info.closeTime2().ticks());
}

static TimeDelta timeDeltaFromTicks(int64_t ticks) {
    int64_t days = ticks / 86400000000LL;
    int64_t remain = ticks - days * 86400000000LL;
    int64_t hours = remain / 3600000000LL;
    remain -= hours * 3600000000LL;
    int64_t minutes = remain / 60000000LL;
    remain -= minutes * 60000000LL;
    int64_t seconds = remain / 1000000LL;
    remain -= seconds * 1000000LL;
    return TimeDelta(days, hours, minutes, seconds, 0, remain);
}

MarketInfo decodeMarketInfo(Reader& rd) {
    string market = rd.getString();
    string name = rd.getString();
    string description = rd.getString();
    string code = rd.getString();
    Datetime lastDate = rd.getDatetime();
    TimeDelta openTime1 = timeDeltaFromTicks(rd.getI64());
    TimeDelta closeTime1 = timeDeltaFromTicks(rd.getI64());
    TimeDelta openTime2 = timeDeltaFromTicks(rd.getI64());
    TimeDelta closeTime2 = timeDeltaFromTicks(rd.getI64());
    HKU_IF_RETURN(!rd.ok(), MarketInfo());
    return MarketInfo(market, name, description, code, lastDate, openTime1, closeTime1, openTime2,
                      closeTime2);
}

///////////////////////////////////////////////////////////////////////////////
// StockTypeInfo
///////////////////////////////////////////////////////////////////////////////
void encodeStockTypeInfo(Encoder& enc, const StockTypeInfo& info) {
    enc.putU32(info.type());
    enc.putString(info.description());
    enc.putDouble(info.tick());
    enc.putDouble(info.tickValue());
    enc.putDouble(info.unit());
    enc.putI32(info.precision());
    enc.putDouble(info.minTradeNumber());
    enc.putDouble(info.maxTradeNumber());
}

StockTypeInfo decodeStockTypeInfo(Reader& rd) {
    uint32_t type = rd.getU32();
    string description = rd.getString();
    price_t tick = rd.getDouble();
    price_t tickValue = rd.getDouble();
    rd.getDouble();  // unit，可由 tickValue / tick 导出，忽略
    int precision = rd.getI32();
    double minTradeNumber = rd.getDouble();
    double maxTradeNumber = rd.getDouble();
    HKU_IF_RETURN(!rd.ok(), StockTypeInfo());
    return StockTypeInfo(type, description, tick, tickValue, precision, minTradeNumber,
                         maxTradeNumber);
}

///////////////////////////////////////////////////////////////////////////////
// ZhBond10
///////////////////////////////////////////////////////////////////////////////
void encodeZhBond10(Encoder& enc, const ZhBond10& bond) {
    enc.putDatetime(bond.date);
    enc.putDouble(bond.value);
}

ZhBond10 decodeZhBond10(Reader& rd) {
    ZhBond10 bond;
    bond.date = rd.getDatetime();
    bond.value = rd.getDouble();
    return bond;
}

void encodeZhBond10List(Encoder& enc, const ZhBond10List& bonds) {
    enc.putU64(bonds.size());
    for (const auto& bond : bonds) {
        encodeZhBond10(enc, bond);
    }
}

ZhBond10List decodeZhBond10List(Reader& rd) {
    ZhBond10List bonds;
    // 单条国债线上最小 16 字节：date(8) + value(8)
    uint64_t count = rd.getCount(16);
    HKU_IF_RETURN(!rd.ok(), bonds);
    bonds.resize(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        bonds[i] = decodeZhBond10(rd);
    }
    return bonds;
}

///////////////////////////////////////////////////////////////////////////////
// HistoryFinanceInfo
///////////////////////////////////////////////////////////////////////////////
void encodeHistoryFinanceInfo(Encoder& enc, const HistoryFinanceInfo& info) {
    enc.putDatetime(info.fileDate);
    enc.putDatetime(info.reportDate);
    enc.putU32(static_cast<uint32_t>(info.values.size()));
    for (float v : info.values) {
        enc.putFloat(v);
    }
}

HistoryFinanceInfo decodeHistoryFinanceInfo(Reader& rd) {
    HistoryFinanceInfo info;
    info.fileDate = rd.getDatetime();
    info.reportDate = rd.getDatetime();
    // 单个财务字段为 float，线上 4 字节
    uint32_t count = rd.getCount32(4);
    HKU_IF_RETURN(!rd.ok(), info);
    info.values.resize(count);
    for (uint32_t i = 0; i < count && rd.ok(); i++) {
        info.values[i] = rd.getFloat();
    }
    return info;
}

void encodeHistoryFinanceList(Encoder& enc, const std::vector<HistoryFinanceInfo>& infos) {
    enc.putU64(infos.size());
    for (const auto& info : infos) {
        encodeHistoryFinanceInfo(enc, info);
    }
}

std::vector<HistoryFinanceInfo> decodeHistoryFinanceList(Reader& rd) {
    std::vector<HistoryFinanceInfo> infos;
    // 单条财务信息线上最小 20 字节：fileDate(8) + reportDate(8) + values 个数(4)
    uint64_t count = rd.getCount(20);
    HKU_IF_RETURN(!rd.ok(), infos);
    infos.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        infos.emplace_back(decodeHistoryFinanceInfo(rd));
    }
    return infos;
}

///////////////////////////////////////////////////////////////////////////////
// 节假日与财务字段
///////////////////////////////////////////////////////////////////////////////
void encodeHolidaySet(Encoder& enc, const std::unordered_set<Datetime>& holidays) {
    enc.putU64(holidays.size());
    for (const auto& d : holidays) {
        enc.putDatetime(d);
    }
}

std::unordered_set<Datetime> decodeHolidaySet(Reader& rd) {
    std::unordered_set<Datetime> holidays;
    // 单个节假日线上 8 字节
    uint64_t count = rd.getCount(8);
    HKU_IF_RETURN(!rd.ok(), holidays);
    holidays.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        holidays.insert(rd.getDatetime());
    }
    return holidays;
}

void encodeFinanceField(Encoder& enc, const std::vector<std::pair<size_t, std::string>>& fields) {
    enc.putU64(fields.size());
    for (const auto& field : fields) {
        enc.putU64(field.first);
        enc.putString(field.second);
    }
}

std::vector<std::pair<size_t, std::string>> decodeFinanceField(Reader& rd) {
    std::vector<std::pair<size_t, std::string>> fields;
    // 单个字段线上最小 10 字节：序号(8) + 名称字符串前缀(2)
    uint64_t count = rd.getCount(10);
    HKU_IF_RETURN(!rd.ok(), fields);
    fields.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        size_t ix = static_cast<size_t>(rd.getU64());
        string name = rd.getString();
        fields.emplace_back(ix, std::move(name));
    }
    return fields;
}

///////////////////////////////////////////////////////////////////////////////
// Block
///////////////////////////////////////////////////////////////////////////////
void encodeBlock(Encoder& enc, const Block& block) {
    enc.putString(block.category());
    enc.putString(block.name());
    Stock index_stock = block.getIndexStock();
    enc.putString(index_stock.isNull() ? "" : index_stock.market_code());
    auto stocks = block.getStockList();
    enc.putU64(stocks.size());
    for (const auto& stk : stocks) {
        enc.putString(stk.market_code());
    }
}

Block decodeBlock(Reader& rd) {
    string category = rd.getString();
    string name = rd.getString();
    string index_code = rd.getString();
    Block block(category, name);
    // 单个成分股代码线上最小 2 字节（字符串长度前缀）
    uint64_t count = rd.getCount(2);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        block.add(rd.getString());
    }
    if (!index_code.empty()) {
        Stock index_stock = StockManager::instance().getStock(index_code);
        if (!index_stock.isNull()) {
            block.setIndexStock(index_stock);
        }
    }
    return block;
}

void encodeBlockList(Encoder& enc, const BlockList& blocks) {
    enc.putU64(blocks.size());
    for (const auto& block : blocks) {
        encodeBlock(enc, block);
    }
}

BlockList decodeBlockList(Reader& rd) {
    BlockList blocks;
    // 单个板块线上最小 14 字节：3 个字符串前缀(6) + 成分股个数(8)
    uint64_t count = rd.getCount(14);
    HKU_IF_RETURN(!rd.ok(), blocks);
    blocks.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        blocks.emplace_back(decodeBlock(rd));
    }
    return blocks;
}

///////////////////////////////////////////////////////////////////////////////
// KQuery
///////////////////////////////////////////////////////////////////////////////
void encodeKQuery(Encoder& enc, const KQuery& query) {
    enc.putU8(static_cast<uint8_t>(query.queryType()));
    enc.putString(query.kType());
    if (query.queryType() == KQuery::INDEX) {
        enc.putI64(query.start());
        enc.putI64(query.end());
    } else {
        enc.putDatetime(query.startDatetime());
        enc.putDatetime(query.endDatetime());
    }
}

KQuery decodeKQuery(Reader& rd) {
    uint8_t query_type = rd.getU8();
    string ktype = rd.getString();
    HKU_IF_RETURN(!rd.ok(), KQuery());
    if (query_type == KQuery::INDEX) {
        int64_t start = rd.getI64();
        int64_t end = rd.getI64();
        HKU_IF_RETURN(!rd.ok(), KQuery());
        return KQuery(start, end, ktype);
    }
    Datetime start = rd.getDatetime();
    Datetime end = rd.getDatetime();
    HKU_IF_RETURN(!rd.ok(), KQuery());
    return KQueryByDate(start, end, ktype);
}

///////////////////////////////////////////////////////////////////////////////
// Parameter(string -> double)
///////////////////////////////////////////////////////////////////////////////
void encodeParamMap(Encoder& enc, const std::unordered_map<std::string, double>& params) {
    enc.putU64(params.size());
    for (const auto& item : params) {
        enc.putString(item.first);
        enc.putDouble(item.second);
    }
}

std::unordered_map<std::string, double> decodeParamMap(Reader& rd) {
    std::unordered_map<std::string, double> params;
    // 单个参数线上最小 10 字节：键字符串前缀(2) + 值(8)
    uint64_t count = rd.getCount(10);
    HKU_IF_RETURN(!rd.ok(), params);
    params.reserve(count);
    for (uint64_t i = 0; i < count && rd.ok(); i++) {
        string key = rd.getString();
        double value = rd.getDouble();
        params.emplace(std::move(key), value);
    }
    return params;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
