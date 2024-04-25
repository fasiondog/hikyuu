/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-01-03
 *      Author: fasiondog
 */

#include "hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h"
#include "hikyuu/utilities/db_connect/TableMacro.h"
#include "SQLiteBlockInfoDriver.h"

namespace hku {

struct SQLiteBlockTable {
    TABLE_BIND4(SQLiteBlockTable, block, category, name, market_code, index_code)
    string category;
    string name;
    string market_code;
    string index_code;
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
    connect.batchLoadView(records,
                          "select a.id, a.category, a.name, a.market_code, b.market_code as "
                          "index_code from block a left "
                          "join BlockIndex b on a.category=b.category and a.name = b.name");

    for (auto& record : records) {
        auto category_iter = m_buffer.find(record.category);
        if (category_iter == m_buffer.end()) {
            m_buffer[record.category] = {};
        }
        auto& name_dict = m_buffer[record.category];
        auto name_iter = name_dict.find(record.name);
        if (name_iter == name_dict.end()) {
            name_dict[record.name] = {Block(record.category, record.name, record.index_code)};
        }
        name_dict[record.name].add(record.market_code);
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