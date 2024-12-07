/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-01-03
 *      Author: fasiondog
 */

#include "hikyuu/utilities/db_connect/DBConnect.h"
#include "hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h"
#include "SQLiteBlockInfoDriver.h"

namespace hku {

struct SQLiteBlockView {
    TABLE_BIND4(SQLiteBlockView, block, category, name, market_code, index_code)
    string category;
    string name;
    string market_code;
    string index_code;
};

struct SQLiteBlockTable {
    TABLE_BIND3(SQLiteBlockTable, block, category, name, market_code)
    string category;
    string name;
    string market_code;
};

struct SQLiteBlockIndexTable {
    TABLE_BIND3(SQLiteBlockIndexTable, BlockIndex, category, name, market_code)
    string category;
    string name;
    string market_code;
};

SQLiteBlockInfoDriver::~SQLiteBlockInfoDriver() {}

bool SQLiteBlockInfoDriver::_init() {
    string dbname = tryGetParam<string>("db", "");
    return !(dbname == "");
}

DBConnectPtr SQLiteBlockInfoDriver::getConnect() {
    string dbname = tryGetParam<string>("db", "");
    HKU_CHECK(!dbname.empty(), "Can't get Sqlite3 filename!");
    HKU_TRACE("SQLITE3: {}", dbname);
    return std::make_shared<SQLiteConnect>(m_params);
}

void SQLiteBlockInfoDriver::load() {
    vector<SQLiteBlockView> records;
    auto connect = getConnect();
    connect->batchLoadView(records,
                           "select a.id, a.category, a.name, a.market_code, b.market_code as "
                           "index_code from block a left "
                           "join BlockIndex b on a.category=b.category and a.name = b.name");

    std::unique_lock<std::shared_mutex> lock(m_buffer_mutex);
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
    std::shared_lock<std::shared_mutex> lock(m_buffer_mutex);
    auto category_iter = m_buffer.find(category);
    HKU_IF_RETURN(category_iter == m_buffer.end(), ret);

    auto block_iter = category_iter->second.find(name);
    HKU_IF_RETURN(block_iter == category_iter->second.end(), ret);

    ret = block_iter->second;
    return ret;
}

BlockList SQLiteBlockInfoDriver::getBlockList(const string& category) {
    BlockList ret;
    std::shared_lock<std::shared_mutex> lock(m_buffer_mutex);
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
    std::shared_lock<std::shared_mutex> lock(m_buffer_mutex);
    for (auto category_iter = m_buffer.begin(); category_iter != m_buffer.end(); ++category_iter) {
        const auto& category_blocks = category_iter->second;
        for (auto iter = category_blocks.begin(); iter != category_blocks.end(); ++iter) {
            ret.emplace_back(iter->second);
        }
    }
    return ret;
}

void SQLiteBlockInfoDriver::save(const Block& block) {
    std::unique_lock<std::shared_mutex> lock(m_buffer_mutex);
    auto category_iter = m_buffer.find(block.category());
    if (category_iter == m_buffer.end()) {
        m_buffer.emplace(block.category(), unordered_map<string, Block>{{block.name(), block}});
    } else {
        category_iter->second.emplace(block.name(), block);
    }

    auto connect = getConnect();
    AutoTransAction trans(connect);
    auto condition = (Field("category") == block.category()) & (Field("name") == block.name());
    connect->remove(SQLiteBlockView::getTableName(), condition, false);
    connect->remove(SQLiteBlockIndexTable::getTableName(), condition, false);

    if (!block.getIndexStock().isNull()) {
        SQLiteBlockIndexTable index;
        index.category = block.category();
        index.name = block.name();
        index.market_code = block.getIndexStock().market_code();
        connect->save(index, false);
    }

    for (auto iter = block.begin(); iter != block.end(); ++iter) {
        SQLiteBlockTable record;
        record.category = block.category();
        record.name = block.name();
        record.market_code = iter->market_code();
        connect->save(record, false);
    }
}

void SQLiteBlockInfoDriver::remove(const string& category, const string& name) {
    {
        auto connect = getConnect();
        AutoTransAction trans(connect);
        auto condition = (Field("category") == category) & (Field("name") == name);
        connect->remove(SQLiteBlockTable::getTableName(), condition, false);
        connect->remove(SQLiteBlockIndexTable::getTableName(), condition, false);
    }

    std::unique_lock<std::shared_mutex> lock(m_buffer_mutex);
    auto category_iter = m_buffer.find(category);
    HKU_IF_RETURN(category_iter == m_buffer.end(), void());

    auto block_iter = category_iter->second.find(name);
    HKU_IF_RETURN(block_iter == category_iter->second.end(), void());

    category_iter->second.erase(block_iter);
    m_buffer.erase(category_iter);
}

}  // namespace hku