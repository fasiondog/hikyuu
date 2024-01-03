/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240102 added by fasiondog
 */

#include <nlohmann/json.hpp>
#include "hikyuu/utilities/db_connect/mysql/MySQLConnect.h"
#include "hikyuu/utilities/db_connect/TableMacro.h"
#include "MySQLBlockInfoDriver.h"

using json = nlohmann::json;

namespace hku {

struct MySQLBlockTable {
    TABLE_BIND2(MySQLBlockTable, block, category, content)
    string category;
    string content;
};

MySQLBlockInfoDriver::~MySQLBlockInfoDriver() {}

bool MySQLBlockInfoDriver::_init() {
    return true;
}

void MySQLBlockInfoDriver::load() {
    Parameter connect_param;
    connect_param.set<string>("host", getParamFromOther<string>(m_params, "host", "127.0.0.1"));
    connect_param.set<string>("usr", getParamFromOther<string>(m_params, "usr", "root"));
    connect_param.set<string>("pwd", getParamFromOther<string>(m_params, "pwd", ""));
    connect_param.set<string>("db", getParamFromOther<string>(m_params, "db", "hku_base"));
    string port_str = getParamFromOther<string>(m_params, "port", "3306");
    unsigned int port = boost::lexical_cast<unsigned int>(port_str);
    connect_param.set<int>("port", port);
    MySQLConnect connect(connect_param);

    vector<MySQLBlockTable> records;
    connect.batchLoad(records);

    unordered_map<string, Block> tmp;
    for (const auto& record : records) {
        json blks = json::parse(record.content);
        for (json::iterator it = blks.begin(); it != blks.end(); ++it) {
            Block blk(record.category, it.key());
            for (const auto& codes : it.value()) {
                blk.add(codes);
            }
            tmp[it.key()] = blk;
        }
        m_buffer[record.category] = std::move(tmp);
        tmp.clear();
    }
}

Block MySQLBlockInfoDriver::getBlock(const string& category, const string& name) {
    Block ret;
    auto category_iter = m_buffer.find(category);
    HKU_IF_RETURN(category_iter == m_buffer.end(), ret);

    auto block_iter = category_iter->second.find(name);
    HKU_IF_RETURN(block_iter == category_iter->second.end(), ret);

    ret = block_iter->second;
    return ret;
}

BlockList MySQLBlockInfoDriver::getBlockList(const string& category) {
    BlockList ret;
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
    for (auto category_iter = m_buffer.begin(); category_iter != m_buffer.end(); ++category_iter) {
        const auto& category_blocks = category_iter->second;
        for (auto iter = category_blocks.begin(); iter != category_blocks.end(); ++iter) {
            ret.emplace_back(iter->second);
        }
    }
    return ret;
}

}  // namespace hku