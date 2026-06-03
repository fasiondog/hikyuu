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
     * 只指定因子名称 (+ k线类型)，将尝试自动从数据库加载因子
     * @param name 因子名称
     * @param ktype K线类型
     */
    explicit Factor(const string& name, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * 构造新因子(因子名称 + K线类型 为因子的唯一标识)
     * @param name 因子名称
     * @param formula 计算公式指标, 一旦创建不可更改
     * @param ktype K线类型
     * @param brief 简要描述
     * @param details 详细描述
     * @param start_date 因子数据起始日期
     * @param save_value 是否需要持久化保存因子值数据
     * @param block 板块信息，证券集合，如果为空，为全部
     * @param recover_type 复权方式
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
     * 获取指定股票的指定查询参数的计算结果
     * @param stock 证券
     * @param query 查询参数
     * @param align 是否对齐日期（按指定 align_dates 或默认交易日历)，默认 false
     * @param fill_null 是否填充空值，默认 false
     * @param tovalue 是否转换为数值，默认 false
     * @param check 是否检查股票属于自身指定的 block，默认 false
     * @param align_dates 对齐日期列表，默认为空
     * @return 计算结果指标
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
     * 获取指定股票列表的指定查询参数的计算结果
     * @param stocks 证券列表
     * @param query 查询参数
     * @param align 是否对齐日期（按指定align_dates或默认交易日历)，默认 false
     * @param fill_null 是否填充空值，默认 false
     * @param tovalue 是否转换为数值，默认 false
     * @param check 是否检查股票列表属于自身指定的 block，默认 false
     * @param align_dates 对齐日期列表，默认为空
     * @return 按股票顺序排列的计算结果列表
     */
    IndicatorList getValues(const StockList& stocks, const KQuery& query, bool align = false,
                            bool fill_null = false, bool tovalue = false, bool check = false,
                            const DatetimeList& align_dates = {}) const;

    /**
     * 获取指定查询参数的所有计算结果
     * @param query 查询参数
     * @param align 是否对齐日期（按指定align_dates或默认交易日历)，默认 false
     * @param fill_null 是否填充空值，默认 false
     * @param tovalue 是否转换为数值，默认 false
     * @param align_dates 对齐日期列表，默认为空
     * @return 所有股票的计算结果列表
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
     * 保存因子即其所有计算结果到数据库，如果因子已存在则更新，否则插入新记录
     * @note 因子名称不区分大小写，以 name + ktype 作为唯一标识
     * @param update_before 是否在保存前，检查并更新已有因子，默认
     * true。注意：通常必须为true，否则会导致数据错误，除非你确定所有因子值都已更新
     */
    void save_to_db(bool update_before = true);

    /**
     * 特殊因子保存值到数据库, 其值不是不通过指标计算，如: PRICELIST，需要自行指定设置
     */
    void save_special_values_to_db(const Stock& stock, const DatetimeList& dates,
                                   const PriceList& values, bool replace = false);

    void save_special_values_to_db(const Stock& stock, const Indicator& values,
                                   bool replace = false);

    /**
     * 从数据库中删除因子及其数据, 注：为防止误操作，特殊因子的值不会删除，需自行手工删除
     */
    void remove_from_db();

    /**
     * 从数据库中加载因子，以 name + ktype 作为唯一标识，如果不存在则不修改当前对象
     */
    void load_from_db();

private:
    struct Data {
        string name;                  ///< 因子名称
        string ktype;                 ///< K线类型
        string brief;                 ///< 简要描述
        string details;               ///< 详细描述
        Datetime create_at;           ///< 创建时间
        Datetime update_at;           ///< 更新时间
        Datetime start_date;          ///< 开始日期，数据存储时的起始日期
        Indicator formula;            ///< 计算公式指标
        Block block;                  ///< 板块信息，证券集合，如果为空，为全部
        bool need_save_value{false};  ///< 是否需要持久化保存因子值数据
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