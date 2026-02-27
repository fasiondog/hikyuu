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
     * @param need_persist 是否需要持久化
     */
    Factor(const string& name, const Indicator& formula, const KQuery::KType& ktype = KQuery::DAY,
           const string& brief = "", const string& details = "", bool need_persist = false,
           const Datetime& start_date = Datetime::min(), const Block& block = Block());

    Factor(const Factor& other);
    Factor(Factor&& other);
    ~Factor() = default;

    Factor& operator=(const Factor& other);
    Factor& operator=(Factor&& other);

    /**
     * 获取指定股票列表的指定查询参数的计算结果
     * @param stocks 证券列表
     * @param query 查询参数
     * @param check 是否检查股票列表属于自身指定的 block
     */
    IndicatorList getValues(const StockList& stocks, const KQuery& query, bool align = false,
                            bool fill_null = false, bool tovalue = false, bool check = false) const;

    /**
     * 获取指定查询参数的所有计算结果
     * @param query 获取指定查询参数的计算结果
     */
    IndicatorList getAllValues(const KQuery& query, bool align = false, bool fill_null = false,
                               bool tovalue = false);

    const string& name() const noexcept;

    void name(const string& name);

    const string& ktype() const noexcept;

    void ktype(const string& ktype);

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

    bool needPersist() const noexcept;

    void needPersist(bool flag);

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
     */
    void save_to_db();

    /**
     * 从数据库中删除因子及其数据
     */
    void remove_from_db();

    /**
     * 从数据库中加载因子，以 name + ktype 作为唯一标识，如果不存在则不修改当前对象
     */
    void load_from_db();

private:
    struct Data {
        string name;               ///< 因子名称
        string ktype;              ///< K线类型
        string brief;              ///< 简要描述
        string details;            ///< 详细描述
        Datetime create_at;        ///< 创建时间
        Datetime update_at;        ///< 更新时间
        Datetime start_date;       ///< 开始日期，数据存储时的起始日期
        Indicator formula;         ///< 计算公式指标
        Block block;               ///< 板块信息，证券集合，如果为空，为全部
        bool need_persist{false};  ///< 是否需要持久化

        Data() = default;
        Data(const string& name, const Indicator& formula, const KQuery::KType& ktype,
             const string& brief, const string& details, bool need_persist,
             const Datetime& start_date, const Block& block)
        : name(name),
          ktype(ktype),
          brief(brief),
          details(details),
          start_date(start_date),
          formula(formula.clone()),
          block(block),
          need_persist(need_persist) {
            to_upper(this->name);
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
        string name = this->name();
        ar& BOOST_SERIALIZATION_NVP(name);
        string ktype = this->ktype();
        ar& BOOST_SERIALIZATION_NVP(ktype);
        Indicator formula = this->formula();
        ar& BOOST_SERIALIZATION_NVP(formula);
        Datetime startDate = this->startDate();
        ar& BOOST_SERIALIZATION_NVP(startDate);
        Block block = this->block();
        ar& BOOST_SERIALIZATION_NVP(block);
        Datetime createAt = this->createAt();
        ar& BOOST_SERIALIZATION_NVP(createAt);
        Datetime updateAt = this->updateAt();
        ar& BOOST_SERIALIZATION_NVP(updateAt);
        string brief = this->brief();
        ar& BOOST_SERIALIZATION_NVP(brief);
        string details = this->details();
        ar& BOOST_SERIALIZATION_NVP(details);
        bool needPersist = this->needPersist();
        ar& BOOST_SERIALIZATION_NVP(needPersist);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        string name;
        string ktype;
        Indicator formula;
        Datetime startDate;
        Block block;
        Datetime createAt;
        Datetime updateAt;
        string brief;
        string details;
        bool needPersist;
        ar& BOOST_SERIALIZATION_NVP(name);
        ar& BOOST_SERIALIZATION_NVP(ktype);
        ar& BOOST_SERIALIZATION_NVP(formula);
        ar& BOOST_SERIALIZATION_NVP(startDate);
        ar& BOOST_SERIALIZATION_NVP(block);
        ar& BOOST_SERIALIZATION_NVP(createAt);
        ar& BOOST_SERIALIZATION_NVP(updateAt);
        ar& BOOST_SERIALIZATION_NVP(brief);
        ar& BOOST_SERIALIZATION_NVP(details);
        ar& BOOST_SERIALIZATION_NVP(needPersist);
        this->m_data =
          make_shared<Data>(name, formula, ktype, brief, details, needPersist, startDate, block);
        this->createAt(createAt);
        this->updateAt(updateAt);
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

inline void Factor::name(const string& name) {
    m_data->name = name;
    to_upper(m_data->name);
    m_data->formula.name(m_data->name);
}

inline const string& Factor::ktype() const noexcept {
    return m_data->ktype;
}

inline void Factor::ktype(const string& ktype) {
    m_data->ktype = ktype;
}

inline const Indicator& Factor::formula() const noexcept {
    return m_data->formula;
}

inline void Factor::formula(const Indicator& formula) {
    m_data->formula = formula.clone();
    m_data->formula.name(m_data->name);
    m_data->formula.setContext(KData());
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

inline bool Factor::needPersist() const noexcept {
    return m_data->need_persist;
}

inline void Factor::needPersist(bool flag) {
    m_data->need_persist = flag;
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