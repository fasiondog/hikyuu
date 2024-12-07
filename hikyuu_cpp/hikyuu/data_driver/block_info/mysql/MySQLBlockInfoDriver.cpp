/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240102 added by fasiondog
 */

#include "hikyuu/utilities/db_connect/mysql/MySQLConnect.h"
#include "hikyuu/utilities/db_connect/DBConnect.h"
#include "MySQLBlockInfoDriver.h"

namespace hku {

struct MySQLBlockView {
    TABLE_BIND4(MySQLBlockView, block, category, name, market_code, index_code)
    string category;
    string name;
    string market_code;
    string index_code;
};

struct MySQLBlockTable {
    TABLE_BIND3(MySQLBlockTable, block, category, name, market_code)
    string category;
    string name;
    string market_code;
};

struct MySQLBlockIndexTable {
    TABLE_BIND3(MySQLBlockIndexTable, BlockIndex, category, name, market_code)
    string category;
    string name;
    string market_code;
};

MySQLBlockInfoDriver::~MySQLBlockInfoDriver() {}

bool MySQLBlockInfoDriver::_init() {
    return true;
}

DBConnectPtr MySQLBlockInfoDriver::getConnect() {
    Parameter connect_param;
    connect_param.set<string>("host", getParamFromOther<string>(m_params, "host", "127.0.0.1"));
    connect_param.set<string>("usr", getParamFromOther<string>(m_params, "usr", "root"));
    connect_param.set<string>("pwd", getParamFromOther<string>(m_params, "pwd", ""));
    connect_param.set<string>("db", getParamFromOther<string>(m_params, "db", "hku_base"));
    string port_str = getParamFromOther<string>(m_params, "port", "3306");
    unsigned int port = boost::lexical_cast<unsigned int>(port_str);
    connect_param.set<int>("port", port);
    return std::make_shared<MySQLConnect>(connect_param);
}

void MySQLBlockInfoDriver::load() {
    auto connect = getConnect();
    vector<MySQLBlockView> records;
    connect->batchLoadView(
      records,
      "select a.id, a.category, a.name, a.market_code, b.market_code as "
      "index_code from `hku_base`.`block` a left "
      "join `hku_base`.`BlockIndex` b on a.category=b.category and a.name = b.name");

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

Block MySQLBlockInfoDriver::getBlock(const string& category, const string& name) {
    Block ret;
    std::shared_lock<std::shared_mutex> lock(m_buffer_mutex);
    auto category_iter = m_buffer.find(category);
    HKU_IF_RETURN(category_iter == m_buffer.end(), ret);

    auto block_iter = category_iter->second.find(name);
    HKU_IF_RETURN(block_iter == category_iter->second.end(), ret);

    ret = block_iter->second;
    return ret;
}

BlockList MySQLBlockInfoDriver::getBlockList(const string& category) {
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

BlockList MySQLBlockInfoDriver::getBlockList() {
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

void MySQLBlockInfoDriver::save(const Block& block) {
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
    connect->remove(MySQLBlockView::getTableName(), condition, false);
    connect->remove(MySQLBlockIndexTable::getTableName(), condition, false);

    if (!block.getIndexStock().isNull()) {
        MySQLBlockIndexTable index;
        index.category = block.category();
        index.name = block.name();
        index.market_code = block.getIndexStock().market_code();
        connect->save(index, false);
    }

    for (auto iter = block.begin(); iter != block.end(); ++iter) {
        MySQLBlockTable record;
        record.category = block.category();
        record.name = block.name();
        record.market_code = iter->market_code();
        connect->save(record, false);
    }
}

void MySQLBlockInfoDriver::remove(const string& category, const string& name) {
    {
        auto connect = getConnect();
        AutoTransAction trans(connect);
        auto condition = (Field("category") == category) & (Field("name") == name);
        connect->remove(MySQLBlockTable::getTableName(), condition, false);
        connect->remove(MySQLBlockIndexTable::getTableName(), condition, false);
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