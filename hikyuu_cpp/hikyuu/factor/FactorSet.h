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

    /**
     * 构造函数，使用指定的指标列表创建因子集合，因子名称默认为指标名称, 主要用于创建临时的因子集合
     * @note 同名的指标会被覆盖，最终保留最后一个同名指标
     * @param inds 指标列表
     * @param ktype 因子集合的 K 线类型，默认为日线
     */
    explicit FactorSet(const IndicatorList& inds, const KQuery::KType& ktype = KQuery::DAY);

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
                                    bool align = false, bool fill_null = false,
                                    bool tovalue = false, bool check = false) const;

    /**
     * 获取所有因子的指定查询参数的计算结果
     * @param query 查询参数
     * @return 所有因子的计算结果
     */
    vector<IndicatorList> getAllValues(const KQuery& query, bool align = false,
                                       bool fill_null = false, bool tovalue = false) const;

    const string& name() const noexcept;

    void name(const string& name);

    const string& ktype() const noexcept;

    void ktype(const string& ktype);

    const Block& block() const noexcept;

    void block(const Block& blk);

    //------------------------
    // 容器操作接口
    //------------------------

    size_t size() const noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

    bool isNull() const noexcept;

    string str() const;

    //------------------------
    // 因子管理接口
    //------------------------

    void add(const Factor& factor);
    void add(Factor&& factor);
    void add(const FactorList&);
    void add(const Indicator& ind);
    void add(const IndicatorList& inds);

    void remove(const string& name);
    bool have(const string& name) const noexcept;

    const Factor& get(const string& name) const;
    const Factor& get(size_t i) const;

    const Factor& operator[](const string& name) const;

    const Factor& operator[](size_t i) const;

    const FactorList& getAllFactors() const;

    void save_to_db() const;
    void remove_from_db() const;
    void load_from_db();

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

        explicit const_iterator(const typename vector<Factor>::const_iterator& iter)
        : m_iter(iter) {}

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

        // const_iterator operator++(int) {
        //     const_iterator temp = *this;
        //     ++m_iter;
        //     return temp;
        // }

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

    const_iterator begin() const;

    const_iterator end() const;

    const_iterator cbegin() const;

    const_iterator cend() const;

private:
    struct HKU_API Data {
        string name;
        string ktype;
        Block block;
        vector<Factor> factors;                      // 保持插入顺序
        unordered_map<string, size_t> nameIndexMap;  // 名称到索引的映射，用于快速查找
    };
    shared_ptr<Data> m_data;

private:
    static shared_ptr<Data> ms_null_factorset;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        string name = this->name();
        ar& BOOST_SERIALIZATION_NVP(name);
        string ktype = this->ktype();
        ar& BOOST_SERIALIZATION_NVP(ktype);
        Block block = this->block();
        ar& BOOST_SERIALIZATION_NVP(block);
        FactorList factors = this->getAllFactors();
        ar& BOOST_SERIALIZATION_NVP(factors);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        string name;
        string ktype;
        Block block;
        ar& BOOST_SERIALIZATION_NVP(name);
        ar& BOOST_SERIALIZATION_NVP(ktype);
        ar& BOOST_SERIALIZATION_NVP(block);
        FactorList factors;
        ar& BOOST_SERIALIZATION_NVP(factors);
        this->m_data = make_shared<Data>();
        this->m_data->name = name;
        this->m_data->ktype = ktype;
        this->m_data->block = block;
        for (auto& factor : factors) {
            this->add(std::move(factor));
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

///////////////////////////////////////////////////////////////////////////////
// inline impl
///////////////////////////////////////////////////////////////////////////////

inline const string& FactorSet::name() const noexcept {
    return m_data->name;
}

inline void FactorSet::name(const string& name) {
    m_data->name = name;
    to_upper(m_data->name);
}

inline const string& FactorSet::ktype() const noexcept {
    return m_data->ktype;
}

inline void FactorSet::ktype(const string& ktype) {
    for (auto& factor : m_data->factors) {
        HKU_CHECK(factor.ktype() == ktype, "ktype not match for factor '{}'", factor.name());
    }
    m_data->ktype = ktype;
}

inline const Block& FactorSet::block() const noexcept {
    return m_data->block;
}

inline void FactorSet::block(const Block& blk) {
    m_data->block = blk;
}

inline size_t FactorSet::size() const noexcept {
    return m_data->factors.size();
}

inline bool FactorSet::empty() const noexcept {
    return m_data->factors.empty();
}

inline void FactorSet::clear() noexcept {
    m_data->factors.clear();
    m_data->nameIndexMap.clear();
}

inline bool FactorSet::isNull() const noexcept {
    return m_data == ms_null_factorset;
}

inline const Factor& FactorSet::get(size_t i) const {
    return m_data->factors[i];
}

inline const Factor& FactorSet::operator[](const string& name) const {
    return get(name);
}

inline const Factor& FactorSet::operator[](size_t i) const {
    return m_data->factors[i];
}

inline const FactorList& FactorSet::getAllFactors() const {
    return m_data->factors;
}

inline FactorSet::const_iterator FactorSet::begin() const {
    return const_iterator(m_data->factors.begin());
}

inline FactorSet::const_iterator FactorSet::end() const {
    return const_iterator(m_data->factors.end());
}

inline FactorSet::const_iterator FactorSet::cbegin() const {
    return const_iterator(m_data->factors.cbegin());
}

inline FactorSet::const_iterator FactorSet::cend() const {
    return const_iterator(m_data->factors.cend());
}

typedef vector<FactorSet> FactorSetList;

HKU_API std::ostream& operator<<(std::ostream& os, const FactorSet&);

}  // namespace hku