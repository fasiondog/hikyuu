/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"
#include "hikyuu/Stock.h"
#include "hikyuu/global/SpotRecord.h"

namespace hku {

void HKU_API startDataServer(const std::string& addr = "tcp://0.0.0.0:9201", size_t work_num = 2,
                             bool save_tick = false, bool buf_tick = false);

void HKU_API stopDataServer();

/**
 * 从 dataserver 数据缓存服务器拉取更新最新的缓存数据
 * @param addr 缓存服务地址，如: tcp://192.168.1.1:9201
 * @param stklist 待更新的股票列表
 * @param ktype 指定更新的K线类型
 */
void HKU_API getDataFromBufferServer(const std::string& addr, const StockList& stklist,
                                     const KQuery::KType& ktype);

/**
 * @brief 从 dataserver 获取指定证券大于等于指定日期的缓存 spot 数据
 * @param addr 缓存服务地址，如: tcp://192.168.1.1:9201
 * @param market 市场代码
 * @param code 证券代码
 * @param datetime 查询时间
 * @return vector<SpotRecord>
 */
vector<SpotRecord> HKU_API getSpotFromBufferServer(const std::string& addr,
                                                   const std::string& market,
                                                   const std::string& code,
                                                   const Datetime& datetime);

}  // namespace hku