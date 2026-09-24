/*
 * Block.h
 *
 *  Created on: 2015-02-08
 *      Author: fasiondog
 */

#pragma once
#ifndef BLOCK_H_
#define BLOCK_H_

#include "StockMapIterator.h"

namespace hku {

/**
 * Sector (Block) class, which can be regarded as a container of securities
 * @ingroup StockManage
 */
class HKU_API Block {
public:
    Block() noexcept;
    Block(const string& category, const string& name);
    Block(const string& category, const string& name, const string& indexCode);
    Block(const Block&) noexcept;
    Block(Block&&) noexcept;
    Block& operator=(const Block&) noexcept;
    Block& operator=(Block&&) noexcept;
    virtual ~Block();

    /** Create a sector from a security list; the category and name are empty, usually used for a
     *  temporary sector */
    explicit Block(const StockList& stocks);

    /** Create a sector from a security code list; the category and name are empty, usually used for
     *  a temporary sector */
    explicit Block(const StringList& market_codes);

    typedef StockMapIterator const_iterator;
    const_iterator begin() const {
        const_iterator iter;
        if (m_data)
            iter = StockMapIterator(m_data->m_stockDict.begin());
        return iter;
    }

    const_iterator end() const {
        const_iterator iter;
        if (m_data)
            iter = StockMapIterator(m_data->m_stockDict.end());
        return iter;
    }

    bool isNull() const noexcept {
        return !m_data;
    }

    uint64_t id() const noexcept {
        return m_data ? (uint64_t)m_data.get() : 0;
    }

    bool operator==(const Block& blk) const noexcept;

    bool operator!=(const Block& blk) const noexcept {
        return !(*this == blk);
    }

    /** Get the sector category */
    string category() const noexcept {
        return m_data ? m_data->m_category : "";
    }

    /** Get the sector name */
    string name() const noexcept {
        return m_data ? m_data->m_name : "";
    }

    /** Set the sector category */
    void category(const string& category) {
        if (!m_data)
            m_data = make_shared<Data>();
        m_data->m_category = category;
    }

    /** Set the name */
    void name(const string& name) {
        if (!m_data)
            m_data = make_shared<Data>();
        m_data->m_name = name;
    }

    /** Whether the given security is contained */
    bool have(const string& market_code) const;

    /** Whether the given security is contained */
    bool have(const Stock& stock) const;

    /** Get the given security */
    Stock get(const string& market_code) const;

    /** Get the given security */
    Stock operator[](const string& market_code) const {
        return get(market_code);
    }

    /** Get all the securities in the sector */
    StockList getStockList(
      std::function<bool(const Stock&)>&& filter = std::function<bool(const Stock&)>()) const;

    /** Add the given security */
    bool add(const Stock& stock);

    /** Add the given security */
    bool add(const string& market_code);

    /**
     * Add the given security list
     * @param stocks security list
     * @return true all succeeded
     * @return false at least one failed
     */
    bool add(const StockList& stocks);

    /**
     * Add the given security list
     * @param market_codes security identifier list
     * @return true all succeeded
     * @return false at least one failed
     */
    bool add(const StringList& market_codes);

    /** Remove the given security */
    bool remove(const string& market_code);

    /** Remove the given security */
    bool remove(const Stock& stock);

    /** Number of contained securities */
    size_t size() const noexcept {
        return m_data ? m_data->m_stockDict.size() : 0;
    }

    /** Whether it is empty */
    bool empty() const noexcept {
        return size() == 0;
    }

    /** Remove all contained securities */
    void clear() {
        if (m_data)
            m_data->m_stockDict.clear();
    }

    /** Get the corresponding index; it may be a null Stock */
    Stock getIndexStock() const noexcept {
        return m_data ? m_data->m_indexStock : Stock();
    }

    /** Set the corresponding index */
    void setIndexStock(const Stock& stk);

    uint64_t strongHash() const;

private:
    struct HKU_API Data {
        string m_category;
        string m_name;
        Stock m_indexStock;  // The corresponding index, which may not exist
        StockMapIterator::stock_map_t m_stockDict;
    };
    shared_ptr<Data> m_data;
};

/** @ingroup StockManage */
typedef vector<Block> BlockList;

HKU_API std::ostream& operator<<(std::ostream& os, const Block&);

/**
 * @brief Get a Block from StockManager
 * @param category
 * @param name
 * @return HKU_API
 */
HKU_API Block getBlock(const string& category, const string& name);

} /* namespace hku */

namespace std {
template <>
class hash<hku::Block> {
public:
    size_t operator()(hku::Block const& blk) const noexcept {
        return blk.id();
    }
};
}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Block> : ostream_formatter {};
#endif

#endif /* BLOCK_H_ */
