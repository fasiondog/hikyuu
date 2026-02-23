/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-13
 *      Author: fasiondog
 */

#pragma once

#include "Factor.h"
#include "hikyuu/Block.h"

namespace hku {

class HKU_API FactorSet {
public:
    FactorSet();
    explicit FactorSet(const string& name, const KQuery::KType& ktype = KQuery::DAY,
                       const Block& block = Block());
    FactorSet(const FactorSet& other);
    FactorSet(FactorSet&& other);
    virtual ~FactorSet() = default;

    FactorSet& operator=(const FactorSet& other);
    FactorSet& operator=(FactorSet&& other);

    /**
     * 获取指定证券列表的指定查询参数的计算结果
     * @param stocks 证券列表
     * @param query 查询参数
     * @param check 是否检查股票列表属于自身指定的 block
     * @return stocks * inds 的列表, 按证券顺序
     */
    vector<IndicatorList> getValues(const StockList& stocks, const KQuery& query,
                                    bool check = false) const;

    /**
     * 获取所有因子的指定查询参数的计算结果
     * @param query 查询参数
     * @return 所有因子的计算结果
     */
    vector<IndicatorList> getAllValues(const KQuery& query) const;

    //------------------------
    // 基本属性
    //------------------------

    const string& name() const noexcept {
        return m_data->name;
    }

    const string& ktype() const noexcept {
        return m_data->ktype;
    }

    const Block& block() const noexcept {
        return m_data->block;
    }

    void block(const Block& blk) {
        m_data->block = blk;
    }

    //------------------------
    // 容器操作接口
    //------------------------

    size_t size() const noexcept {
        return m_data->m_factors.size();
    }

    bool empty() const noexcept {
        return m_data->m_factors.empty();
    }

    void clear() noexcept {
        m_data->m_factors.clear();
        m_data->m_nameIndexMap.clear();
    }

    bool isNull() const noexcept {
        return m_data == ms_null_factorset;
    }

    string str() const;

    //------------------------
    // 因子管理接口
    //------------------------

    void add(const Factor& factor);
    void add(Factor&& factor);
    void remove(const string& name);
    bool have(const string& name) const noexcept;

    const Factor& get(const string& name) const;
    const Factor& get(size_t i) const {
        return m_data->m_factors[i];
    }

    const Factor& operator[](const string& name) const {
        return get(name);
    }

    const Factor& operator[](size_t i) const {
        return m_data->m_factors[i];
    }

    const FactorList& getAllFactors() const {
        return m_data->m_factors;
    }

    //------------------------
    // 迭代器支持
    //------------------------

    // 迭代器类定义
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const Factor;
        using difference_type = std::ptrdiff_t;
        using pointer = const Factor*;
        using reference = const Factor&;

        const_iterator(const typename vector<Factor>::const_iterator& iter) : m_iter(iter) {}

        reference operator*() const {
            return *m_iter;
        }

        pointer operator->() const {
            return &(*m_iter);
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
        typename vector<Factor>::const_iterator m_iter;
    };

    using iterator = const_iterator;

    const_iterator begin() const {
        return const_iterator(m_data->m_factors.begin());
    }

    const_iterator end() const {
        return const_iterator(m_data->m_factors.end());
    }

    const_iterator cbegin() const {
        return const_iterator(m_data->m_factors.cbegin());
    }

    const_iterator cend() const {
        return const_iterator(m_data->m_factors.cend());
    }

private:
    struct HKU_API Data {
        string name;
        string ktype;
        Block block;
        vector<Factor> m_factors;                      // 保持插入顺序
        unordered_map<string, size_t> m_nameIndexMap;  // 名称到索引的映射，用于快速查找
    };
    shared_ptr<Data> m_data;

private:
    static shared_ptr<Data> ms_null_factorset;
};

typedef vector<FactorSet> FactorSetList;

HKU_API std::ostream& operator<<(std::ostream& os, const FactorSet&);

}  // namespace hku