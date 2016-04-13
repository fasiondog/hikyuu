/*
 * Block.h
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#ifndef BLOCK_H_
#define BLOCK_H_

#include "StockMapIterator.h"

namespace hku {

class HKU_API Block {
public:
    Block();
    Block(const string& category, const string& name);
    Block(const Block&);
    Block& operator=(const Block&);
    virtual ~Block();

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

    bool operator==(const Block& blk) {
        return m_data == blk.m_data;
    }

    bool operator!=(const Block& blk) {
        return m_data != blk.m_data;
    }

    string category() const {
        return m_data ? m_data->m_category : "";
    }

    string name() const {
        return m_data ? m_data->m_name : "";
    }

    void category(const string& category) {
        if (!m_data)
            m_data = shared_ptr<Data>(new Data);
        m_data->m_category = category;
    }

    void name(const string& name) {
        if (!m_data)
            m_data = shared_ptr<Data>(new Data);
        m_data->m_name = name;
    }

    bool have(const string& market_code) const;
    bool have(const Stock& stock) const;

    Stock get(const string&) const;
    Stock operator[](const string& market_code) const {
        return get(market_code);
    }
    bool add(const Stock& stock);
    bool add(const string&);
    bool remove(const string&);
    bool remove(const Stock& stock);

    size_t size() const {
        return m_data ? m_data->m_stockDict.size() : 0;
    }

    bool empty() const {
        return size() ? false : true;
    }

    void clear() {
        if (m_data) m_data->m_stockDict.clear();
    }

private:
    struct HKU_API Data {
        string m_category;
        string m_name;
        StockMapIterator::stock_map_t m_stockDict;
    };
    shared_ptr<Data> m_data;
};

typedef vector<Block> BlockList;

HKU_API std::ostream& operator <<(std::ostream &os, const Block&);

} /* namespace hku */

#endif /* BLOCK_H_ */
