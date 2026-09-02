/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"
#include "hikyuu/StockWeight.h"
#include "hikyuu/TimeLineRecord.h"
#include "hikyuu/TransRecord.h"
#include "hikyuu/MarketInfo.h"
#include "hikyuu/StockTypeInfo.h"
#include "hikyuu/ZhBond10.h"
#include "hikyuu/HistoryFinanceInfo.h"
#include "hikyuu/Block.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"

namespace hku {

/**
 * IPC 数据服务二进制协议
 * @details 单机同架构、小端字节序。
 * 请求帧：[magic u32][version u16][cmd u16][body_len u32][body ...]
 * 响应帧：[magic u32][ret i32][body_len u32][body ...]
 * @ingroup DataDriver
 */
namespace ipc {

constexpr uint32_t MAGIC = 0x53444B48;  // "HKDS" 小端
constexpr uint16_t VERSION = 1;
constexpr size_t REQUEST_HEADER_SIZE = 12;
constexpr size_t RESPONSE_HEADER_SIZE = 12;

/** 响应状态码 */
enum class RetCode : int32_t {
    SUCCESS = 0,
    ERROR = -1,
    LOADING = 1,  // 服务端数据仍在加载中，携带当前进度
};

/** 命令字 */
enum class Cmd : uint16_t {
    STATUS_READY = 0,

    BASE_ALL_STOCK_INFO,
    BASE_STOCK_INFO,
    BASE_ALL_MARKET_INFO,
    BASE_ALL_STOCK_TYPE_INFO,
    BASE_ALL_HOLIDAYS,
    BASE_ALL_ZHBOND10,
    BASE_STOCK_WEIGHT_LIST,
    BASE_ALL_STOCK_WEIGHT_LIST,
    BASE_HISTORY_FINANCE_FIELD,
    BASE_HISTORY_FINANCE,
    BASE_FINANCE_INFO,

    KDATA_COUNT,
    KDATA_INDEX_RANGE_BY_DATE,
    KDATA_GET_KRECORD_LIST,
    KDATA_GET_TIMELINE_LIST,
    KDATA_GET_TRANS_LIST,

    BLOCK_LOAD,

    STATUS_SHM_INFO,  // 查询 K 线共享内存缓存段信息（名称/代数）
};

/** 编码缓冲，按需增长 */
class HKU_API Encoder {
public:
    Encoder() = default;

    const std::vector<uint8_t>& data() const noexcept {
        return m_buf;
    }

    size_t size() const noexcept {
        return m_buf.size();
    }

    void putU8(uint8_t v);
    void putU16(uint16_t v);
    void putU32(uint32_t v);
    void putU64(uint64_t v);
    void putI32(int32_t v);
    void putI64(int64_t v);
    void putDouble(double v);
    void putFloat(float v);
    void putString(const std::string& s);
    void putDatetime(const Datetime& d);

private:
    std::vector<uint8_t> m_buf;
};

/** 解码读取器，越界时标记失败 */
class HKU_API Reader {
public:
    Reader(const uint8_t* data, size_t len) : m_data(data), m_len(len) {}

    bool ok() const noexcept {
        return m_ok;
    }

    size_t remain() const noexcept {
        return m_ok && m_pos < m_len ? m_len - m_pos : 0;
    }

    uint8_t getU8();
    uint16_t getU16();
    uint32_t getU32();
    uint64_t getU64();
    int32_t getI32();
    int64_t getI64();
    double getDouble();
    float getFloat();
    std::string getString();
    Datetime getDatetime();

    /**
     * 读取集合元素个数（u64），并校验其不超过剩余字节所能容纳的上限
     * @details 线上 count 来自对端，坏帧或版本错配可能给出天文数字，
     * 调用方直接 resize/reserve 会触发 length_error/bad_alloc。超界时置失败，
     * 由调用方按既有的 rd.ok() 判定回退。
     * @param elem_min_size 单个元素编码后的最小字节数
     */
    uint64_t getCount(size_t elem_min_size);

    /** 同 getCount，但元素个数字段为 u32 */
    uint32_t getCount32(size_t elem_min_size);

private:
    bool check(size_t n) noexcept;

    const uint8_t* m_data;
    size_t m_len;
    size_t m_pos{0};
    bool m_ok{true};
};

///@{ 帧编解码
/** 编码请求帧 */
HKU_API std::vector<uint8_t> encodeRequest(Cmd cmd, const std::vector<uint8_t>& body);

/** 解码请求帧 */
HKU_API bool decodeRequest(const std::vector<uint8_t>& frame, Cmd& out_cmd,
                           std::vector<uint8_t>& out_body);

/** 编码响应帧 */
HKU_API std::vector<uint8_t> encodeResponse(RetCode ret, const std::vector<uint8_t>& body);

/** 解码响应帧 */
HKU_API bool decodeResponse(const std::vector<uint8_t>& frame, RetCode& out_ret,
                            std::vector<uint8_t>& out_body);
///@}

///@{ 记录编解码
HKU_API void encodeKRecord(Encoder& enc, const KRecord& k);
HKU_API KRecord decodeKRecord(Reader& rd);
HKU_API void encodeKRecordList(Encoder& enc, const KRecordList& ks);
HKU_API KRecordList decodeKRecordList(Reader& rd);

HKU_API void encodeTimeLineRecord(Encoder& enc, const TimeLineRecord& t);
HKU_API TimeLineRecord decodeTimeLineRecord(Reader& rd);
HKU_API void encodeTimeLineList(Encoder& enc, const TimeLineList& ts);
HKU_API TimeLineList decodeTimeLineList(Reader& rd);

HKU_API void encodeTransRecord(Encoder& enc, const TransRecord& t);
HKU_API TransRecord decodeTransRecord(Reader& rd);
HKU_API void encodeTransList(Encoder& enc, const TransList& ts);
HKU_API TransList decodeTransList(Reader& rd);

HKU_API void encodeStockWeight(Encoder& enc, const StockWeight& w);
HKU_API StockWeight decodeStockWeight(Reader& rd);
HKU_API void encodeStockWeightList(Encoder& enc, const StockWeightList& ws);
HKU_API StockWeightList decodeStockWeightList(Reader& rd);

HKU_API void encodeStockInfo(Encoder& enc, const StockInfo& info);
HKU_API StockInfo decodeStockInfo(Reader& rd);
HKU_API void encodeStockInfoList(Encoder& enc, const std::vector<StockInfo>& infos);
HKU_API std::vector<StockInfo> decodeStockInfoList(Reader& rd);

HKU_API void encodeMarketInfo(Encoder& enc, const MarketInfo& info);
HKU_API MarketInfo decodeMarketInfo(Reader& rd);

HKU_API void encodeStockTypeInfo(Encoder& enc, const StockTypeInfo& info);
HKU_API StockTypeInfo decodeStockTypeInfo(Reader& rd);

HKU_API void encodeZhBond10(Encoder& enc, const ZhBond10& bond);
HKU_API ZhBond10 decodeZhBond10(Reader& rd);
HKU_API void encodeZhBond10List(Encoder& enc, const ZhBond10List& bonds);
HKU_API ZhBond10List decodeZhBond10List(Reader& rd);

HKU_API void encodeHistoryFinanceInfo(Encoder& enc, const HistoryFinanceInfo& info);
HKU_API HistoryFinanceInfo decodeHistoryFinanceInfo(Reader& rd);
HKU_API void encodeHistoryFinanceList(Encoder& enc, const std::vector<HistoryFinanceInfo>& infos);
HKU_API std::vector<HistoryFinanceInfo> decodeHistoryFinanceList(Reader& rd);

HKU_API void encodeHolidaySet(Encoder& enc, const std::unordered_set<Datetime>& holidays);
HKU_API std::unordered_set<Datetime> decodeHolidaySet(Reader& rd);

HKU_API void encodeFinanceField(Encoder& enc,
                                const std::vector<std::pair<size_t, std::string>>& fields);
HKU_API std::vector<std::pair<size_t, std::string>> decodeFinanceField(Reader& rd);

HKU_API void encodeBlock(Encoder& enc, const Block& block);
HKU_API Block decodeBlock(Reader& rd);
HKU_API void encodeBlockList(Encoder& enc, const BlockList& blocks);
HKU_API BlockList decodeBlockList(Reader& rd);

HKU_API void encodeKQuery(Encoder& enc, const KQuery& query);
HKU_API KQuery decodeKQuery(Reader& rd);

/** Parameter 仅支持 string->double 键值对（用于财务信息传输） */
HKU_API void encodeParamMap(Encoder& enc, const std::unordered_map<std::string, double>& params);
HKU_API std::unordered_map<std::string, double> decodeParamMap(Reader& rd);
///@}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
