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
     * Constructor, create a factor set with the given factor list
     * @param factors factor list
     * @param ktype K-line type, the daily line by default
     * @param block block, empty by default
     * @param name factor set name, empty by default
     */
    explicit FactorSet(const FactorList& factors, const KQuery::KType& ktype = KQuery::DAY,
                       const Block& block = Block(), const string& name = "");

    /**
     * Constructor, create a factor set with the given indicator list, the factor name is the
     * indicator name by default; it is mainly used to create a temporary factor set
     * @note An indicator with the same name is overwritten, the last one with the same name is kept
     * @param inds indicator list
     * @param ktype K-line type of the factor set, the daily line by default
     */
    explicit FactorSet(const IndicatorList& inds, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Constructor, create a factor set with the given indicator map
     * @note An indicator with the same name is overwritten, the last one with the same name is kept
     * @param inds indicator map, the key is the factor name and the value is the corresponding
     *             indicator
     * @param ktype K-line type of the factor set, the daily line by default
     */
    explicit FactorSet(const std::unordered_map<string, Indicator>& inds,
                       const KQuery::KType& ktype = KQuery::DAY);

    FactorSet(const FactorSet& other);
    FactorSet(FactorSet&& other);
    virtual ~FactorSet() = default;

    FactorSet& operator=(const FactorSet& other);
    FactorSet& operator=(FactorSet&& other);

    /**
     * Get the calculation results of the given query condition on the given stock list
     * @param stocks security list
     * @param query query condition
     * @param align whether to align the dates (according to the given align_dates or the default
     *              trading calendar), false by default
     * @param fill_null whether to fill the null values, false by default
     * @param tovalue whether to convert to values, false by default
     * @param check whether to check that the stock list belongs to the block specified by itself,
     *              false by default
     * @param align_dates the align date list, empty by default
     * @return the list of stocks * inds, in the security order
     */
    vector<IndicatorList> getValues(const StockList& stocks, const KQuery& query,
                                    bool align = false, bool fill_null = false,
                                    bool tovalue = false, bool check = false,
                                    const DatetimeList& align_dates = {}) const;

    /**
     * Get the calculation results of the given query condition on all the factors
     * @param query query condition
     * @param align whether to align the dates (according to the given align_dates or the default
     *              trading calendar), false by default
     * @param fill_null whether to fill the null values, false by default
     * @param tovalue whether to convert to values, false by default
     * @param align_dates the align date list, empty by default
     * @return the calculation results of all the factors
     */
    vector<IndicatorList> getAllValues(const KQuery& query, bool align = false,
                                       bool fill_null = false, bool tovalue = false,
                                       const DatetimeList& align_dates = {}) const;

    const string& name() const noexcept;

    void name(const string& name);

    const string& ktype() const noexcept;

    void ktype(const string& ktype);

    const Block& block() const noexcept;

    void block(const Block& blk);

    //------------------------
    // Container operation interface
    //------------------------

    size_t size() const noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

    bool isNull() const noexcept;

    string str() const;

    //------------------------
    // Factor management interface
    //------------------------

    void add(const Factor& factor);
    void add(const FactorList& factors);

    /** Convenience method: add an indicator and use the given name as the factor name */
    void add(const string& name, const Indicator& ind);

    /** Convenience method: add an indicator and use the indicator name as the factor name. Note:
     *  the later indicator with the same name overwrites the one added before */
    void add(const Indicator& ind);

    void add(const IndicatorList& inds);
    void add(const std::map<string, Indicator>& inds);

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
    // Iterator support
    //------------------------

    // Iterator class definition
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
        string ktype{KQuery::DAY};
        Block block;
        vector<Factor> factors;                      // Keep the insertion order
        unordered_map<string, size_t> nameIndexMap;  // Name to index mapping, used for fast lookup
    };
    shared_ptr<Data> m_data;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        string tmp_name = name();
        ar& BOOST_SERIALIZATION_NVP(tmp_name);
        string tmp_ktype = ktype();
        ar& BOOST_SERIALIZATION_NVP(tmp_ktype);
        Block tmp_block = block();
        ar& BOOST_SERIALIZATION_NVP(tmp_block);
        FactorList tmp_factors = getAllFactors();
        ar& BOOST_SERIALIZATION_NVP(tmp_factors);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        string tmp_name;
        string tmp_ktype;
        Block tmp_block;
        ar& BOOST_SERIALIZATION_NVP(tmp_name);
        ar& BOOST_SERIALIZATION_NVP(tmp_ktype);
        ar& BOOST_SERIALIZATION_NVP(tmp_block);
        FactorList tmp_factors;
        ar& BOOST_SERIALIZATION_NVP(tmp_factors);
        m_data = make_shared<Data>();
        m_data->name = tmp_name;
        m_data->ktype = tmp_ktype;
        m_data->block = tmp_block;
        for (auto& factor : tmp_factors) {
            add(std::move(factor));
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
    m_data->name = utf8_to_upper(name);
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
    return !m_data;
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