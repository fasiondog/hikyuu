/*
 * Block.cpp
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#include "xxhash.h"
#include "StockManager.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Block& blk) {
    string strip(", ");
    os << "Block(" << blk.category() << strip << blk.name() << strip << blk.size() << ")";
    return os;
}

Block::Block() noexcept {}

Block::~Block() {}

Block::Block(const string& category, const string& name) : m_data(make_shared<Data>()) {
    m_data->m_category = category;
    m_data->m_name = name;
}

Block::Block(const string& category, const string& name, const string& indexCode)
: Block(category, name) {
    if (!indexCode.empty()) {
        auto stock = StockManager::instance().getStock(indexCode);
        if (!stock.isNull()) {
            m_data->m_indexStock = stock;
        } else {
            // 直接忽略不再打印
            HKU_TRACE("Can't find index stock: {}, will ignore!", indexCode);
        }
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

Block::Block(const StockList& stocks) : m_data(make_shared<Data>()) {
    for (const auto& stock : stocks) {
        add(stock);
    }
}

Block::Block(const StringList& market_codes) : m_data(make_shared<Data>()) {
    for (const auto& market_code : market_codes) {
        add(market_code);
    }
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
    // stock 为空时不打印日志，防止打印过多，尤其是部分累积不用的板块在初始化时会打印很多日志
    HKU_IF_RETURN(stock.isNull() || have(stock), false);
    if (!m_data) [[unlikely]]
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

uint64_t Block::strongHash() const {
    HKU_IF_RETURN(!m_data, 0);

    XXH64_state_t* state = XXH64_createState();
    HKU_IF_RETURN(!state, 0);

    uint64_t seed = 0;
    XXH64_reset(state, seed);

    XXH64_update(state, m_data->m_category.data(), m_data->m_category.size());
    XXH64_update(state, m_data->m_name.data(), m_data->m_name.size());

    StockList stocks = getStockList();
    std::sort(stocks.begin(), stocks.end(),
              [](const Stock& a, const Stock& b) { return a.market_code() < b.market_code(); });
    for (const auto& stk : stocks) {
        auto stkid = stk.id();
        XXH64_update(state, &stkid, sizeof(stkid));
    }

    // 获取最终哈希值
    uint64_t result = XXH64_digest(state);
    XXH64_freeState(state);
    return result;
}

bool Block::operator==(const Block& blk) const noexcept {
    HKU_IF_RETURN(this == &blk || m_data == blk.m_data, true);
    HKU_IF_RETURN(category() != blk.category() || name() != blk.name() || size() != blk.size() ||
                    getIndexStock() != blk.getIndexStock(),
                  false);
    unordered_map<string, string> self_dict;
    for (const auto& stk : getStockList()) {
        self_dict[stk.market_code()] = stk.market_code();
    }
    for (const auto& stk : blk.getStockList()) {
        HKU_IF_RETURN(self_dict.count(stk.market_code()) == 0, false);
    }
    return true;
}

HKU_API Block getBlock(const string& category, const string& name) {
    auto& sm = StockManager::instance();
    return sm.getBlock(category, name);
}

} /* namespace hku */
