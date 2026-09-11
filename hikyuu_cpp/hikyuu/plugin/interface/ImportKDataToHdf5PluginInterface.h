/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/KRecord.h"
#include "hikyuu/KQuery.h"
#include "hikyuu/TimeLineRecord.h"
#include "hikyuu/TransRecord.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class ImportKDataToHdf5PluginInterface : public PluginBase {
public:
    ImportKDataToHdf5PluginInterface() = default;
    virtual ~ImportKDataToHdf5PluginInterface() = default;

    /**
     * @brief 设定 K 线数据保存路径及 baseinfo 库文件（sqlite3）
     * @param datapath HDF5 K 线数据保存目录
     * @param markets 需初始化 h5 文件的市场列表
     * @param ktypes 需初始化的 K 线类型列表（DAY/MIN/MIN5/TIMELINE/TRANSDATA）
     * @param baseinfo_path baseinfo sqlite3 库文件路径；为空则 addMarket/addStockType 不可用
     * @note baseinfo_path 与 K 线 datapath 相互独立，通常指向 ~/.hikyuu/stock.db
     */
    virtual bool setConfig(const string& datapath, const vector<string>& markets,
                           const vector<string>& ktypes, const string& baseinfo_path) = 0;

    virtual Datetime getLastDatetime(const string& market, const string& code,
                                     const KQuery::KType& ktype) = 0;

    virtual void addKRecordList(const string& market, const string& code,
                                const vector<KRecord>& krecords, const KQuery::KType& ktype) = 0;

    virtual void addTimeLineList(const string& market, const string& code,
                                 const TimeLineList& timeline) = 0;

    virtual void addTransList(const string& market, const string& code,
                              const TransRecordList& translist) = 0;

    virtual void updateIndex(const string& market, const string& code,
                             const KQuery::KType& ktype) = 0;

    virtual void remove(const string& market, const string& code, const KQuery::KType& ktype,
                        Datetime start) = 0;

    /**
     * @brief 向 baseinfo 库注册新市场（幂等：已存在则跳过并返回 true）
     * @param market 市场简称（自动转大写）
     * @param name 市场名称
     * @param description 描述（建议注明时区，如 "NASDAQ/UTC-5"）
     * @param index_code 市场代表指数代码（getMarketStock/交易日历依赖 {market}{index_code}）
     * @param last_date 起始日期 yyyymmdd
     * @param open1 上午开盘时间 HHMM
     * @param close1 上午收盘时间 HHMM
     * @param open2 下午开盘时间 HHMM
     * @param close2 下午收盘时间 HHMM
     * @return true 成功或已存在；false 失败（许可证无效、baseinfo 未配置、SQL 异常等）
     * @note 注册后须重启 hikyuu_init() 方生效；须另行导入代表指数 K 线
     */
    virtual bool addMarket(const string& market, const string& name, const string& description,
                           const string& index_code, uint64_t last_date, uint64_t open1,
                           uint64_t close1, uint64_t open2, uint64_t close2) = 0;

    /**
     * @brief 向 baseinfo 库注册证券类型（幂等：已存在则跳过并返回 true）
     * @param type_id 类型数值，约定 id == type；仅允许 10(CRYPTO 复用) 或 >= 12（自定义）
     * @param description 类型描述
     * @param precision 价格精度（小数位）
     * @param tick 最小跳动量
     * @param tick_value 每个 tick 的价值
     * @param min_trade 每笔最小交易量
     * @param max_trade 每笔最大交易量
     * @return true 成功或已存在；false 失败
     * @note 注册后须重启 hikyuu_init() 方生效
     */
    virtual bool addStockType(uint32_t type_id, const string& description, uint32_t precision,
                              double tick, double tick_value, double min_trade,
                              double max_trade) = 0;
};

}  // namespace hku
