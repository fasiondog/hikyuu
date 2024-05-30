/*
 * Block.cpp
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#include "StockManager.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Block& blk) {
    string strip(", ");
    os << "Block(" << blk.category() << strip << blk.name() << ")";
    return os;
}

Block::Block() noexcept {}

Block::~Block() {}

Block::Block(const string& category, const string& name) noexcept : m_data(make_shared<Data>()) {
    m_data->m_category = category;
    m_data->m_name = name;
}

Block::Block(const string& category, const string& name, const string& indexCode) noexcept
: Block(category, name) {
    if (!indexCode.empty()) {
        m_data->m_indexStock = StockManager::instance().getStock(indexCode);
    }
}

Block::Block(const Block& block) noexcept {
    if (!block.m_data)
        return;
    m_data = block.m_data;
}

Block::Block(Block&& block) noexcept {
    if (!block.m_data)
        return;
    m_data = std::move(block.m_data);
}

Block& Block::operator=(const Block& block) noexcept {
    HKU_IF_RETURN(this == &block || m_data == block.m_data, *this);
    m_data = block.m_data;
    return *this;
}

Block& Block::operator=(Block&& block) noexcept {
    HKU_IF_RETURN(this == &block || m_data == block.m_data, *this);
    m_data = std::move(block.m_data);
    return *this;
}

bool Block::have(const string& market_code) const {
    HKU_IF_RETURN(!m_data, false);
    string query_str = market_code;
    to_upper(query_str);
    return m_data->m_stockDict.count(query_str) ? true : false;
}

bool Block::have(const Stock& stock) const {
    HKU_IF_RETURN(!m_data, false);
    return m_data->m_stockDict.count(stock.market_code()) ? true : false;
}

Stock Block::get(const string& market_code) const {
    Stock result;
    HKU_IF_RETURN(!m_data, result);
    string query_str = market_code;
    to_upper(query_str);
    auto iter = m_data->m_stockDict.find(query_str);
    if (iter != m_data->m_stockDict.end()) {
        result = iter->second;
    }
    return result;
}

StockList Block::getStockList(std::function<bool(const Stock&)>&& filter) const {
    StockList ret;
    ret.reserve(size());
    auto iter = m_data->m_stockDict.begin();
    if (filter) {
        for (; iter != m_data->m_stockDict.end(); ++iter) {
            if (filter(iter->second)) {
                ret.emplace_back(iter->second);
            }
        }
    } else {
        for (; iter != m_data->m_stockDict.end(); ++iter) {
            ret.emplace_back(iter->second);
        }
    }
    return ret;
}

bool Block::add(const Stock& stock) {
    HKU_IF_RETURN(stock.isNull() || have(stock), false);
    if (!m_data)
        m_data = make_shared<Data>();

    m_data->m_stockDict[stock.market_code()] = stock;
    return true;
}

bool Block::add(const string& market_code) {
    const StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock(market_code);
    HKU_IF_RETURN(stock.isNull() || have(stock), false);
    if (!m_data)
        m_data = make_shared<Data>();

    m_data->m_stockDict[stock.market_code()] = stock;
    return true;
}

bool Block::add(const StockList& stocks) {
    bool success = true;
    for (const auto& stk : stocks) {
        success = add(stk);
    }
    return success;
}

bool Block::add(const StringList& market_codes) {
    bool success = true;
    for (const auto& code : market_codes) {
        success = add(code);
    }
    return success;
}

bool Block::remove(const string& market_code) {
    HKU_IF_RETURN(!have(market_code), false);
    string query_str = market_code;
    to_upper(query_str);
    m_data->m_stockDict.erase(query_str);
    return true;
}

bool Block::remove(const Stock& stock) {
    HKU_IF_RETURN(!have(stock), false);
    m_data->m_stockDict.erase(stock.market_code());
    return true;
}

void Block::setIndexStock(const Stock& stk) {
    if (!m_data)
        m_data = make_shared<Data>();
    m_data->m_indexStock = stk;
}

HKU_API Block getBlock(const string& category, const string& name) {
    auto& sm = StockManager::instance();
    return sm.getBlock(category, name);
}

} /* namespace hku */
