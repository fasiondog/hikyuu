/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-12
 *      Author: fasiondog
 */

#pragma once

#include "FactorImp.h"

namespace hku {

class HKU_API Factor final {
    friend HKU_API std::ostream& operator<<(std::ostream& os, const Factor&);

public:
    Factor();

    /**
     * 尝试加载因子
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

    //------------------------
    // 只读属性
    //------------------------

    const string& name() const noexcept {
        return m_imp->name();
    }

    const string& ktype() const noexcept {
        return m_imp->ktype();
    }

    Indicator formula() const {
        return m_imp->formula();
    }

    const Datetime& startDate() const noexcept {
        return m_imp->startDate();
    }

    const Block& block() const noexcept {
        return m_imp->block();
    }

    //------------------------
    // 可读写属性
    //------------------------

    const Datetime& createAt() const noexcept {
        return m_imp->createAt();
    }

    void createAt(const Datetime& datetime) {
        m_imp->createAt(datetime);
    }

    const Datetime& updateAt() const noexcept {
        return m_imp->updateAt();
    }

    void updateAt(const Datetime& datetime) {
        m_imp->updateAt(datetime);
    }

    bool needPersist() const noexcept {
        return m_imp->needPersist();
    }

    void needPersist(bool flag) {
        m_imp->needPersist(flag);
    }

    const string& brief() const noexcept {
        return m_imp->brief();
    }

    void brief(const string& brief) {
        m_imp->brief(brief);
    }

    const string& details() const noexcept {
        return m_imp->details();
    }

    void details(const string& details) {
        m_imp->details(details);
    }

    //------------------------
    // 其他接口
    //------------------------

    bool isNull() const noexcept {
        return m_imp == ms_null_factor_imp;
    }

    uint64_t hash() const noexcept {
        return m_imp->hash();
    }

    string str() const;

    void save() {
        m_imp->save();
    }

    void remove() {
        m_imp->remove();
    }

    void load() {
        m_imp->load();
    }

    IndicatorList getValues(const StockList& stocks, const KQuery& query) {
        return m_imp->getValues(stocks, query);
    }

    void saveValues(const StockList& stocks, const KQuery& query, bool replace = false) {
        m_imp->saveValues(stocks, query, replace);
    }

private:
    shared_ptr<FactorImp> m_imp;

private:
    static shared_ptr<FactorImp> ms_null_factor_imp;
};

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