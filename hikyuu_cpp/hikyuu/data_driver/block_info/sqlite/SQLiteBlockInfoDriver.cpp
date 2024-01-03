/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-01-03
 *      Author: fasiondog
 */

#include <nlohmann/json.hpp>
#include "hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h"
#include "hikyuu/utilities/db_connect/TableMacro.h"
#include "SQLiteBlockInfoDriver.h"

using json = nlohmann::json;

namespace hku {

struct SQLiteBlockTable {
    TABLE_BIND2(SQLiteBlockTable, block, category, content)
    string category;
    string content;
};

SQLiteBlockInfoDriver::~SQLiteBlockInfoDriver() {}

bool SQLiteBlockInfoDriver::_init() {
    string dbname = tryGetParam<string>("db", "");
    return !(dbname == "");
}

void SQLiteBlockInfoDriver::load() {
    string dbname = tryGetParam<string>("db", "");
    HKU_ERROR_IF_RETURN(dbname == "", void(), "Can't get Sqlite3 filename!");
    HKU_TRACE("SQLITE3: {}", dbname);

    SQLiteConnect connect(m_params);
    vector<SQLiteBlockTable> records;
    connect.batchLoad(records);

    for (const auto& record : records) {
        unordered_map<string, Block> tmp;
        json blks = json::parse(record.content);
        for (json::iterator it = blks.begin(); it != blks.end(); ++it) {
            Block blk(record.category, it.key());
            for (const auto& codes : it.value()) {
                blk.add(codes);
            }
            tmp[it.key()] = blk;
        }
        m_buffer[record.category] = std::move(tmp);
    }
}

Block SQLiteBlockInfoDriver::getBlock(const string& category, const string& name) {
    Block ret;
    auto category_iter = m_buffer.find(category);
    HKU_IF_RETURN(category_iter == m_buffer.end(), ret);

    auto block_iter = category_iter->second.find(name);
    HKU_IF_RETURN(block_iter == category_iter->second.end(), ret);

    ret = block_iter->second;
    return ret;
}

BlockList SQLiteBlockInfoDriver::getBlockList(const string& category) {
    BlockList ret;
    auto category_iter = m_buffer.find(category);
    HKU_IF_RETURN(category_iter == m_buffer.end(), ret);

    const auto& category_blocks = category_iter->second;
    for (auto iter = category_blocks.begin(); iter != category_blocks.end(); ++iter) {
        ret.emplace_back(iter->second);
    }

    return ret;
}

BlockList SQLiteBlockInfoDriver::getBlockList() {
    BlockList ret;
    for (auto category_iter = m_buffer.begin(); category_iter != m_buffer.end(); ++category_iter) {
        const auto& category_blocks = category_iter->second;
        for (auto iter = category_blocks.begin(); iter != category_blocks.end(); ++iter) {
            ret.emplace_back(iter->second);
        }
    }
    return ret;
}

}  // namespace hku