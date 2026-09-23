/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#pragma once

#include <atomic>
#include "hikyuu/KData.h"
#include "ScoresFilterBase.h"
#include "buildin_norm.h"
#include "hikyuu/factor/FactorSet.h"

namespace hku {

/**
 * Multi-factor synthesis; when there is only one factor it is equivalent to a simple score board
 * @ingroup MultiFactor
 */
class HKU_API MultiFactorBase : public enable_shared_from_this<MultiFactorBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    typedef Indicator::value_t value_t;
    friend HKU_API std::ostream& operator<<(std::ostream&, const MultiFactorBase&);

public:
    MultiFactorBase();
    explicit MultiFactorBase(const string& name);
    MultiFactorBase(const StockList& stks, const KQuery& query, const Stock& ref_stk,
                    const string& name, int ic_n, bool spearman, int mode, bool save_all_factors);
    MultiFactorBase(const MultiFactorBase&);
    virtual ~MultiFactorBase() = default;

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /** Set the name */
    void name(const string& name) {
        m_name = name;
    }

    /** Get the reference date list */
    const DatetimeList& getDatetimeList();

    /** Get the query range */
    const KQuery& getQuery() const {
        return m_query;
    }

    /** Set the query range */
    void setQuery(const KQuery& query);

    /** Get the reference security */
    const Stock& getRefStock() const {
        return m_ref_stk;
    }

    /** Set the reference security */
    void setRefStock(const Stock& stk);

    /** Get the security list */
    const StockList& getStockList() const {
        return m_stks;
    }

    /** Set the security list of the calculation range */
    void setStockList(const StockList& stks);

    /** Get the current number of the securities in the security list */
    size_t getStockListNumber() const {
        return m_stks.size();
    }

    /** Get the original factor set */
    const FactorSet& getRefFactorSet() const {
        return m_factorset;
    }

    /** Set the original factor set */
    void setRefFactorSet(const FactorSet& factorset);

    /** Get the synthesized factor of the given security */
    const Indicator& getFactor(const Stock&);

    /**
     * Get the new factors synthesized from all the securities, in the same order as the passed
     * security portfolio
     */
    const IndicatorList& getAllFactors();

    /** Get all the factor values of the given date cross-section, they are already in the
     * descending order */
    ScoreRecordList getScores(const Datetime&);

    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end = Null<size_t>());

    /**
     * Get the factor values (scores) within the given date cross-section range [start, end], and
     * filter them through filter
     * @param date the given date
     * @param start the sorting start point
     * @param end the sorting end point (excluded)
     * @param filter the filter function
     */
    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                              std::function<bool(const ScoreRecord&)>&& filter);

    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                              std::function<bool(const Datetime&, const ScoreRecord&)>&& filter);

    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                              const ScoresFilterPtr& filter);

    /** Get all the cross-section data, they are already in the descending order */
    const vector<ScoreRecordList>& getAllScores();

    /**
     * Get the IC of the synthesized factor, its length is the same as the reference dates (the
     * non-strict IC mode)
     * @note For a new factor using the IC/ICIR weighting, ndays had better stay consistent with
     * ic_n; but for a new factor calculated with the equal weight, ic_n does not have to be used.
     *       Therefore a special value 0 is added to ndays, meaning the IC is calculated directly
     * with the ic_n parameter
     * @param ndays calculate the IC value relative to the ndays day return
     */
    Indicator getIC(int ndays = 0);

    /**
     * Get the ICIR of the synthesized factor
     * @param ir_n the n window for calculating the IR
     * @param ic_n the n window for calculating the IC
     */
    Indicator getICIR(int ir_n, int ic_n = 0);

    /**
     * Get all the processed original factor values (normalized and standardized). It is calculated
     * every time.
     * @note Considering the memory usage, this data is not cached; it is generally used for the
     *       testing or when you want to view the processed original factor values
     * @return vector<IndicatorList>  stks x inds
     */
    vector<IndicatorList> getAllSrcFactors();

    /**
     * Set the factor standardization / normalization operation
     * @param norm the standardization operation
     */
    void setNormalize(NormPtr norm);

    /**
     * Apply the given standardization / normalization, industry neutralization and style factor
     * neutralization operations to the indicator with the given name.
     * @note The standardization, the industry neutralization and the style factor neutralization
     * are independent of each other; they can be given together or separately.
     * @param name indicator name
     * @param norm the standardization operation
     * @param category the block category the indicator belongs to (it needs to be given for the
     *                 neutralization)
     * @param style_inds the style list of the indicator
     */
    void addSpecialNormalize(const string& name, NormalizePtr norm, const string& category = "",
                             const IndicatorList& style_inds = IndicatorList());

    void reset();

    typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
    MultiFactorPtr clone();

    virtual void _reset() {}
    virtual MultiFactorPtr _clone() = 0;
    virtual IndicatorList _calculate(const vector<IndicatorList>&) = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    /**
     * Execute the calculation. It is calculated automatically when the result is got by default.
     *
     * Concurrency semantics (PR1):
     *   - Multiple threads are supported to trigger calculate / getter for the first time on the
     * same uncalculated instance at the same time;
     *   - After a successful calculation, multiple threads are allowed to read concurrently;
     *   - The getter is not supported to be concurrent with
     *     reset/setQuery/setStockList/setRefFactorSet/setParam;
     *   - It is not supported that another thread modifies the instance while the caller holds the
     *     internal reference returned by the getter.
     *
     * Failure semantics:
     *   - When the calculation throws an exception, the derived cache of the base class is cleaned
     * up and m_calculated stays false;
     *   - The exception propagates upward, the next caller can recalculate (DCLP, not
     *     single-flight).
     */
    void calculate();

private:
    void initParam();

    // Clear all the derived data generated after the calculation, keeping the configuration
    // members. It is called before the construction in calculate() and after an exception, ensuring
    // that the retry is based on a clean state.
    void clearCalculatedData();

    // Build the industry attribution labels (integer block indexes) and the block count of every
    // indicator, so that the industry neutralization can be performed.
    // It returns {factor_name -> (labels, blk_count)}:
    //   labels[i] = the index of the block the stock i belongs to in blks (0..blk_count-1), and it
    //   is blk_count if there is no attribution.
    unordered_map<string, std::pair<PriceList, size_t>> _buildDummyIndex();

    void _buildIndex();  // Create the cross-section index after the calculation is finished

    void _checkData();

protected:
    IndicatorList _getAllReturns(int ndays) const;

protected:
    bool m_is_python_object{false};
    string m_name;
    FactorSet m_factorset;  // The input original factor set
    StockList m_stks;       // Security portfolio
    Stock m_ref_stk;        // The given reference security, it is used to align the dates only
    KQuery m_query;         // The date range condition of the calculation

    NormPtr m_norm;                                    // Global standardization / normalization
                                                       // operation
    unordered_map<string, NormPtr> m_special_norms;    // The specific standardization operation
                                                       // performed on a specific indicator
    unordered_map<string, string> m_special_category;  // The block category given when the industry
                                                       // neutralization is performed on a specific
                                                       // indicator
    unordered_map<string, IndicatorList>
      m_special_style_inds;  // The style factors given when the style factor neutralization is
                             // performed on a specific indicator

    // The following variables are generated after the calculation
    DatetimeList m_ref_dates;  // The reference dates calculated from the reference security and the
                               // query, the synthesized factor is aligned to these dates
    unordered_map<Stock, size_t> m_stk_map;  // Security -> the position index of the synthesized
                                             // factor
    IndicatorList m_all_factors;             // Saves the new factors synthesized from all the
                                             // securities
    unordered_map<Datetime, size_t> m_date_index;
    vector<ScoreRecordList> m_stk_factor_by_date;
    Indicator m_ic;

private:
    std::mutex m_mutex;
    std::atomic<bool> m_calculated{false};

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_factorset);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
        ar& BOOST_SERIALIZATION_NVP(m_ref_stk);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_norm);
        ar& BOOST_SERIALIZATION_NVP(m_special_norms);
        ar& BOOST_SERIALIZATION_NVP(m_special_category);
        ar& BOOST_SERIALIZATION_NVP(m_special_style_inds);
        // The following do not need to be saved, they are recalculated after loading
        // ar& BOOST_SERIALIZATION_NVP(m_stk_map);
        // ar& BOOST_SERIALIZATION_NVP(m_all_factors);
        // ar& BOOST_SERIALIZATION_NVP(m_date_index);
        // ar& BOOST_SERIALIZATION_NVP(m_ic);
        // ar& BOOST_SERIALIZATION_NVP(m_calculated);
        // ar& BOOST_SERIALIZATION_NVP(m_stk_factor_by_date);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_factorset);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
        ar& BOOST_SERIALIZATION_NVP(m_ref_stk);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_norm);
        ar& BOOST_SERIALIZATION_NVP(m_special_norms);
        ar& BOOST_SERIALIZATION_NVP(m_special_category);
        ar& BOOST_SERIALIZATION_NVP(m_special_style_inds);
        // ar& BOOST_SERIALIZATION_NVP(m_stk_map);
        // ar& BOOST_SERIALIZATION_NVP(m_all_factors);
        // ar& BOOST_SERIALIZATION_NVP(m_date_index);
        // ar& BOOST_SERIALIZATION_NVP(m_ic);
        // ar& BOOST_SERIALIZATION_NVP(m_calculated);
        // ar& BOOST_SERIALIZATION_NVP(m_stk_factor_by_date);
        m_calculated.store(false, std::memory_order_relaxed);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MultiFactorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public MultiFactorBase {
 *     MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MultiFactor
 */
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                          \
    friend class boost::serialization::access;                    \
    template <class Archive>                                      \
    void serialize(Archive& ar, const unsigned int version) {     \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MultiFactorBase); \
    }
#else
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<MultiFactorBase> FactorPtr;
typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
typedef std::shared_ptr<MultiFactorBase> MFPtr;

#define MULTIFACTOR_IMP(classname)             \
public:                                        \
    virtual MultiFactorPtr _clone() override { \
        return std::make_shared<classname>();  \
    }                                          \
    virtual IndicatorList _calculate(const vector<IndicatorList>&) override;

HKU_API std::ostream& operator<<(std::ostream&, const MultiFactorBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MultiFactorPtr&);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MultiFactorBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::MultiFactorPtr> : ostream_formatter {};
#endif