/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-13
 *      Author: fasiondog
 */

#pragma once

#include "FactorMeta.h"

namespace hku {

class HKU_API FactorSet {
public:
    FactorSet();
    explicit FactorSet(const string& name, const KQuery::KType& ktype = KQuery::DAY);
    FactorSet(const FactorSet& other);
    FactorSet(FactorSet&& other);
    virtual ~FactorSet() = default;

    FactorSet& operator=(const FactorSet& other);
    FactorSet& operator=(FactorSet&& other);

    //------------------------
    // 基本属性
    //------------------------

    const string& name() const noexcept {
        return m_data->name;
    }

    const string& ktype() const noexcept {
        return m_data->ktype;
    }

    //------------------------
    // 容器操作接口
    //------------------------

    size_t size() const noexcept {
        return m_data->m_factorDict.size();
    }

    bool empty() const noexcept {
        return m_data->m_factorDict.empty();
    }

    void clear() noexcept {
        m_data->m_factorDict.clear();
    }

    //------------------------
    // 因子管理接口
    //------------------------

    void addFactor(const FactorMeta& factor);
    void removeFactor(const string& name);
    bool hasFactor(const string& name) const noexcept;
    FactorMeta getFactor(const string& name) const;

    //------------------------
    // 迭代器支持
    //------------------------

    // 迭代器类定义
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const std::pair<const string, FactorMeta>;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::pair<const string, FactorMeta>*;
        using reference = const std::pair<const string, FactorMeta>&;

        const_iterator(const typename std::unordered_map<string, FactorMeta>::const_iterator& iter)
        : m_iter(iter) {}

        reference operator*() const {
            return *m_iter;
        }

        pointer operator->() const {
            return m_iter.operator->();
        }

        const_iterator& operator++() {
            ++m_iter;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++m_iter;
            return temp;
        }

        bool operator==(const const_iterator& other) const {
            return m_iter == other.m_iter;
        }

        bool operator!=(const const_iterator& other) const {
            return m_iter != other.m_iter;
        }

    private:
        typename std::unordered_map<string, FactorMeta>::const_iterator m_iter;
    };

    using iterator = const_iterator;

    const_iterator begin() const {
        return const_iterator(m_data->m_factorDict.begin());
    }

    const_iterator end() const {
        return const_iterator(m_data->m_factorDict.end());
    }

    const_iterator cbegin() const {
        return const_iterator(m_data->m_factorDict.cbegin());
    }

    const_iterator cend() const {
        return const_iterator(m_data->m_factorDict.cend());
    }

private:
    struct HKU_API Data {
        string name;
        string ktype;
        unordered_map<string, FactorMeta> m_factorDict;
    };
    shared_ptr<Data> m_data;
};

}  // namespace hku