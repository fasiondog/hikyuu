/*
 * Block.cpp
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "StockManager.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os, const Block& blk) {
    string strip(", ");
    os << "Block(" << blk.category() << strip << blk.name() << ")";
    return os;
}

Block::Block() {
}

Block::~Block() {
}


Block::Block(const string& category, const string& name) {
    m_data = shared_ptr<Data>(new Data);
    m_data->m_category = category;
    m_data->m_name = name;
}

Block::Block(const Block& block) {
    if (m_data == block.m_data)
        return;
    m_data = block.m_data;
}

Block& Block::operator=(const Block& block) {
    if (this == &block || m_data == block.m_data)
        return *this;

    m_data = block.m_data;
    return *this;
}

bool Block::have(const string& market_code) const {
    if (!m_data) return false;
    string query_str = market_code;
    boost::to_upper(query_str);
    return m_data->m_stockDict.count(query_str) ? true : false;
}

bool Block::have(const Stock& stock) const {
    if (!m_data) return false;
    return m_data->m_stockDict.count(stock.market_code()) ? true : false;
}

Stock Block::get(const string& market_code) const {
    Stock result;
    if (!m_data)
        return result;

    string query_str = market_code;
    boost::to_upper(query_str);
    auto iter = m_data->m_stockDict.find(query_str);
    if (iter != m_data->m_stockDict.end()) {
        result = iter->second;
    }
    return result;
}

bool Block::add(const Stock& stock) {
    if (stock.isNull() || have(stock))
        return false;

    if (!m_data)
        m_data = shared_ptr<Data>(new Data);

    m_data->m_stockDict[stock.market_code()] = stock;
    return true;
}

bool Block::add(const string& market_code) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock(market_code);
    if (stock.isNull() || have(stock))
        return false;

    if (!m_data)
        m_data = shared_ptr<Data>(new Data);

    m_data->m_stockDict[stock.market_code()] = stock;
    return true;
}

bool Block::remove(const string& market_code) {
    if (!have(market_code)) {
        return false;
    }
    string query_str = market_code;
    boost::to_upper(query_str);
    m_data->m_stockDict.erase(query_str);
    return true;
}

bool Block::remove(const Stock& stock) {
    if (!have(stock)) {
        return false;
    }
    m_data->m_stockDict.erase(stock.market_code());
    return true;
}


} /* namespace hku */
