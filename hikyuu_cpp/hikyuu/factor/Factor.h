/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/Block.h"

namespace hku {

class HKU_API Factor final {
    friend HKU_API std::ostream& operator<<(std::ostream& os, const Factor&);

public:
    Factor();

    /**
     * Only the factor name (+ K-line type) is given, it will try to load the factor from the
     * database automatically
     * @param name factor name
     * @param ktype K-line type
     */
    explicit Factor(const string& name, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Construct a new factor (factor name + K-line type is the unique identifier of the factor)
     * @param name factor name
     * @param formula calculation formula indicator, it cannot be changed once created
     * @param ktype K-line type
     * @param brief brief description
     * @param details detailed description
     * @param start_date start date of the factor data
     * @param save_value whether the factor value data needs to be saved persistently
     * @param block block information, a security set; all the securities if it is empty
     * @param recover_type adjustment (recover) type
     */
    Factor(const string& name, const Indicator& formula, const KQuery::KType& ktype = KQuery::DAY,
           const string& brief = "", const string& details = "", bool save_value = false,
           const Datetime& start_date = Datetime::min(), const Block& block = Block(),
           KQuery::RecoverType recover_type = KQuery::RecoverType::NO_RECOVER);

    Factor(const Factor& other) noexcept;
    Factor(Factor&& other) noexcept;
    ~Factor() = default;

    Factor& operator=(const Factor& other) noexcept;
    Factor& operator=(Factor&& other) noexcept;

    /**
     * Get the calculation result of the given query condition on the given stock
     * @param stock security
     * @param query query condition
     * @param align whether to align the dates (according to the given align_dates or the default
     *              trading calendar), false by default
     * @param fill_null whether to fill the null values, false by default
     * @param tovalue whether to convert to values, false by default
     * @param check whether to check that the stock belongs to the block specified by itself, false
     *              by default
     * @param align_dates the align date list, empty by default
     * @return the calculation result indicator
     */
    Indicator getValue(const Stock& stock, const KQuery& query, bool align = false,
                       bool fill_null = false, bool tovalue = false, bool check = false,
                       const DatetimeList& align_dates = {}) const;

    Indicator getValue(const KData& kdata, bool align = false, bool fill_null = false,
                       bool tovalue = false, bool check = false,
                       const DatetimeList& align_dates = {}) const {
        return getValue(kdata.getStock(), kdata.getQuery(), align, fill_null, tovalue, check,
                        align_dates);
    }

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
     * @return the calculation result list arranged in the stock order
     */
    IndicatorList getValues(const StockList& stocks, const KQuery& query, bool align = false,
                            bool fill_null = false, bool tovalue = false, bool check = false,
                            const DatetimeList& align_dates = {}) const;

    /**
     * Get all the calculation results of the given query condition
     * @param query query condition
     * @param align whether to align the dates (according to the given align_dates or the default
     *              trading calendar), false by default
     * @param fill_null whether to fill the null values, false by default
     * @param tovalue whether to convert to values, false by default
     * @param align_dates the align date list, empty by default
     * @return the calculation result list of all the stocks
     */
    IndicatorList getAllValues(const KQuery& query, bool align = false, bool fill_null = false,
                               bool tovalue = false, const DatetimeList& align_dates = {});

    const string& name() const noexcept;

    void name(const string& name);

    const string& ktype() const noexcept;

    void ktype(const string& ktype);

    KQuery::RecoverType recoverType() const noexcept;

    void recoverType(KQuery::RecoverType recover_type);

    const Indicator& formula() const noexcept;

    void formula(const Indicator& formula);

    const Datetime& startDate() const noexcept;

    void startDate(const Datetime& datetime);

    const Block& block() const noexcept;

    void block(const Block& block);

    const Datetime& createAt() const noexcept;

    void createAt(const Datetime& datetime);

    const Datetime& updateAt() const noexcept;

    void updateAt(const Datetime& datetime);

    bool needSaveValue() const noexcept;

    void needSaveValue(bool flag);

    const string& brief() const noexcept;

    void brief(const string& brief);

    const string& details() const noexcept;

    void details(const string& details);

    uint64_t hash() const noexcept;

    bool isNull() const noexcept;

    string str() const;

    /**
     * Save the factor and all its calculation results to the database; the existing factor is
     * updated, otherwise a new record is inserted
     * @note The factor name is case insensitive, name + ktype is used as the unique identifier
     * @param update_before whether to check and update the existing factor before saving, true by
     * default. Note: it usually must be true, otherwise the data will be wrong, unless you are sure
     * that all the factor values have been updated
     */
    void save_to_db(bool update_before = true);

    /**
     * Save the values of a special factor to the database; its values are not obtained through the
     * indicator calculation, such as PRICELIST, so they need to be set manually
     */
    void save_special_values_to_db(const Stock& stock, const DatetimeList& dates,
                                   const PriceList& values, bool replace = false);

    void save_special_values_to_db(const Stock& stock, const Indicator& values,
                                   bool replace = false);

    /**
     * Delete the factor and its data from the database; note: to prevent misoperation, the values
     * of a special factor are not deleted, they need to be deleted manually
     */
    void remove_from_db();

    /**
     * Load the factor from the database, name + ktype is used as the unique identifier; the current
     * object is not modified if it does not exist
     */
    void load_from_db();

private:
    struct Data {
        string name;                  ///< Factor name
        string ktype;                 ///< K-line type
        string brief;                 ///< Brief description
        string details;               ///< Detailed description
        Datetime create_at;           ///< Creation time
        Datetime update_at;           ///< Update time
        Datetime start_date;          ///< Start date, the start date of the data storage
        Indicator formula;            ///< Calculation formula indicator
        Block block;                  ///< Block information, a security set; all if it is empty
        bool need_save_value{false};  ///< Whether the factor value data needs to be saved
                                      ///< persistently
        KQuery::RecoverType recover_type{KQuery::RecoverType::NO_RECOVER};

        Data() = default;
        Data(const string& name, const Indicator& formula, const KQuery::KType& ktype,
             const string& brief, const string& details, bool need_save_value,
             const Datetime& start_date, const Block& block, KQuery::RecoverType recover_type)
        : name(utf8_to_upper(name)),
          ktype(ktype),
          brief(brief),
          details(details),
          start_date(start_date),
          formula(formula.clone()),
          block(block),
          need_save_value(need_save_value),
          recover_type(recover_type) {
            this->formula.setContext(KData());
            this->formula.name(this->name);
            if (this->start_date == Null<Datetime>()) {
                this->start_date = Datetime::min();
            }
        }
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
        Indicator tmp_formula = formula();
        ar& BOOST_SERIALIZATION_NVP(tmp_formula);
        Datetime tmp_startDate = startDate();
        ar& BOOST_SERIALIZATION_NVP(tmp_startDate);
        Block tmp_block = block();
        ar& BOOST_SERIALIZATION_NVP(tmp_block);
        Datetime tmp_createAt = createAt();
        ar& BOOST_SERIALIZATION_NVP(tmp_createAt);
        Datetime tmp_updateAt = updateAt();
        ar& BOOST_SERIALIZATION_NVP(tmp_updateAt);
        string tmp_brief = brief();
        ar& BOOST_SERIALIZATION_NVP(tmp_brief);
        string tmp_details = details();
        ar& BOOST_SERIALIZATION_NVP(tmp_details);
        bool tmp_needSaveValue = needSaveValue();
        ar& BOOST_SERIALIZATION_NVP(tmp_needSaveValue);
        KQuery::RecoverType tmp_recover_type = recoverType();
        ar& BOOST_SERIALIZATION_NVP(tmp_recover_type);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        string tmp_name;
        string tmp_ktype;
        Indicator tmp_formula;
        Datetime tmp_startDate;
        Block tmp_block;
        Datetime tmp_createAt;
        Datetime tmp_updateAt;
        string tmp_brief;
        string tmp_details;
        bool tmp_needSaveValue;
        KQuery::RecoverType tmp_recover_type;
        ar& BOOST_SERIALIZATION_NVP(tmp_name);
        ar& BOOST_SERIALIZATION_NVP(tmp_ktype);
        ar& BOOST_SERIALIZATION_NVP(tmp_formula);
        ar& BOOST_SERIALIZATION_NVP(tmp_startDate);
        ar& BOOST_SERIALIZATION_NVP(tmp_block);
        ar& BOOST_SERIALIZATION_NVP(tmp_createAt);
        ar& BOOST_SERIALIZATION_NVP(tmp_updateAt);
        ar& BOOST_SERIALIZATION_NVP(tmp_brief);
        ar& BOOST_SERIALIZATION_NVP(tmp_details);
        ar& BOOST_SERIALIZATION_NVP(tmp_needSaveValue);
        ar& BOOST_SERIALIZATION_NVP(tmp_recover_type);
        m_data = make_shared<Data>(tmp_name, tmp_formula, tmp_ktype, tmp_brief, tmp_details,
                                   tmp_needSaveValue, tmp_startDate, tmp_block, tmp_recover_type);
        createAt(tmp_createAt);
        updateAt(tmp_updateAt);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

///////////////////////////////////////////////////////////////////////////////
// inline impl
///////////////////////////////////////////////////////////////////////////////

inline const string& Factor::name() const noexcept {
    return m_data->name;
}

inline const string& Factor::ktype() const noexcept {
    return m_data->ktype;
}

inline void Factor::ktype(const string& ktype) {
    m_data->ktype = ktype;
}

inline KQuery::RecoverType Factor::recoverType() const noexcept {
    return m_data->recover_type;
}

inline void Factor::recoverType(KQuery::RecoverType recover_type) {
    m_data->recover_type = recover_type;
}

inline const Indicator& Factor::formula() const noexcept {
    return m_data->formula;
}

inline void Factor::formula(const Indicator& formula) {
    m_data->formula = formula;
    m_data->formula.name(m_data->name);
}

inline const Datetime& Factor::startDate() const noexcept {
    return m_data->start_date;
}

inline void Factor::startDate(const Datetime& datetime) {
    m_data->start_date = datetime == Null<Datetime>() ? Datetime::min() : datetime;
}

inline const Block& Factor::block() const noexcept {
    return m_data->block;
}

inline void Factor::block(const Block& block) {
    m_data->block = block;
}

inline const Datetime& Factor::createAt() const noexcept {
    return m_data->create_at;
}

inline void Factor::createAt(const Datetime& datetime) {
    m_data->create_at = datetime;
}

inline const Datetime& Factor::updateAt() const noexcept {
    return m_data->update_at;
}

inline void Factor::updateAt(const Datetime& datetime) {
    m_data->update_at = datetime;
}

inline const string& Factor::brief() const noexcept {
    return m_data->brief;
}

inline void Factor::brief(const string& brief) {
    m_data->brief = brief;
}

inline const string& Factor::details() const noexcept {
    return m_data->details;
}

inline bool Factor::needSaveValue() const noexcept {
    return m_data->need_save_value;
}

inline void Factor::details(const string& details) {
    m_data->details = details;
}

inline uint64_t Factor::hash() const noexcept {
    return (uint64_t)m_data.get();
}

inline bool Factor::isNull() const noexcept {
    return !m_data || m_data->name.empty() || m_data->ktype.empty();
}

typedef vector<Factor> FactorList;

HKU_API std::ostream& operator<<(std::ostream& os, const Factor&);

}  // namespace hku

namespace std {
template <>
class hash<hku::Factor> {
public:
    size_t operator()(hku::Factor const& factor) const noexcept {
        return factor.hash();
    }
};

}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Factor> : ostream_formatter {};
#endif