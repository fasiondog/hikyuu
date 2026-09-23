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
                             bool save_tick = false, bool buf_tick = false,
                             const std::string& parquet_path = std::string());

void HKU_API stopDataServer();

/**
 * Pull and update the latest cached data from the dataserver cache server
 * @param addr cache service address, e.g. tcp://192.168.1.1:9201
 * @param stklist the stock list to be updated
 * @param ktype the K-line type to be updated
 */
void HKU_API getDataFromBufferServer(const std::string& addr, const StockList& stklist,
                                     const KQuery::KType& ktype);

/**
 * Pull the latest K-lines from the market data cache service and update them in place (the local
 * buffer + the mirror shared memory), without the client routing judgment.
 * @details It is called directly by the IPC handler of the main process only:
 *          getDataFromBufferServer delegates to the main process in the client mode, so the main
 *          process side must go through this function to avoid re-entering the client branch (in
 * the same-process test isIpcClientMode() may be true).
 * @param addr cache service address, e.g. tcp://192.168.1.1:9201
 * @param stklist the stock list to be updated
 * @param ktype the K-line type to be updated
 */
void HKU_API pullFromBufferServerLocal(const std::string& addr, const StockList& stklist,
                                       const KQuery::KType& ktype);

/**
 * @brief Get the cached spot data of the given security from the dataserver, with the date greater
 * than or equal to the given date
 * @param addr cache service address, e.g. tcp://192.168.1.1:9201
 * @param market market code
 * @param code security code
 * @param datetime query time
 * @return vector<SpotRecord>
 */
vector<SpotRecord> HKU_API getSpotFromBufferServer(const std::string& addr,
                                                   const std::string& market,
                                                   const std::string& code,
                                                   const Datetime& datetime);

}  // namespace hku