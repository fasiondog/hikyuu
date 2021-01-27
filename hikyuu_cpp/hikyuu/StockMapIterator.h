/*
 * StockMapIterator.h
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKMAPITERATOR_H_
#define STOCKMAPITERATOR_H_

#include <iterator>
#include "Stock.h"

namespace hku {

class StockMapIterator {
public:
    typedef unordered_map<string, Stock> stock_map_t;
    typedef Stock value_type;
    typedef Stock* pointer;
    typedef const Stock& reference;
    typedef stock_map_t::const_iterator::difference_type difference_type;
    typedef std::input_iterator_tag iterator_category;

    StockMapIterator() {}

    // cppcheck-suppress noExplicitConstructor
    StockMapIterator(const stock_map_t::const_iterator& iter) : m_iter(iter) {}

    StockMapIterator(const StockMapIterator& iter) : m_iter(iter.m_iter) {}

    StockMapIterator& operator=(const StockMapIterator& iter) {
        HKU_IF_RETURN(this == &iter, *this);
        m_iter = iter.m_iter;
        return *this;
    }

    StockMapIterator& operator++() {
        ++m_iter;
        return *this;
    }

    const StockMapIterator operator++(int) {
        auto old_iter = m_iter;
        ++m_iter;
        return StockMapIterator(old_iter);
    }

    bool operator==(const StockMapIterator& iter) const {
        return m_iter == iter.m_iter;
    }

    bool operator!=(const StockMapIterator& iter) const {
        return m_iter != iter.m_iter;
    }

    const Stock& operator*() const {
        return m_iter->second;
    }

    const Stock* const operator->() const {
        return &(m_iter->second);
    }

private:
    stock_map_t::const_iterator m_iter;
};

} /* namespace hku */

#endif /* STOCKMAPITERATOR_H_ */
