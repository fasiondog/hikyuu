/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

/**
 * 因子实现类，封装因子的核心数据和行为
 */
class HKU_API FactorImp {
public:
    /**
     * 默认构造函数
     */
    FactorImp() = default;

    /**
     * 构造函数
     * @param name 因子名称
     * @param formula 计算公式指标
     * @param ktype K线类型
     * @param brief 简要描述
     * @param details 详细描述
     * @param need_persist 是否需要持久化
     */
    FactorImp(const string& name, const Indicator& formula,
              const KQuery::KType& ktype = KQuery::DAY, const string& brief = "",
              const string& details = "", bool need_persist = false);

    virtual ~FactorImp() = default;

    //------------------------
    // 只读属性
    //------------------------

    /**
     * 获取因子名称
     */
    const string& name() const noexcept {
        return m_name;
    }

    /**
     * 获取K线类型
     */
    const string& ktype() const noexcept {
        return m_ktype;
    }

    /**
     * 获取计算公式指标（返回克隆副本）
     */
    Indicator formula() const {
        return m_formula.clone();
    }

    //------------------------
    // 可读写属性
    //------------------------

    /**
     * 获取创建时间
     */
    const Datetime& createAt() const noexcept {
        return m_create_at;
    }

    /**
     * 设置创建时间
     */
    void createAt(const Datetime& datetime) {
        m_create_at = datetime;
    }

    /**
     * 获取更新时间
     */
    const Datetime& updateAt() const noexcept {
        return m_update_at;
    }

    /**
     * 设置更新时间
     */
    void updateAt(const Datetime& datetime) {
        m_update_at = datetime;
    }

    /**
     * 获取是否需要持久化标志
     */
    bool needPersist() const noexcept {
        return m_need_persist;
    }

    /**
     * 设置是否需要持久化标志
     */
    void needPersist(bool flag) {
        m_need_persist = flag;
    }

    /**
     * 获取简要描述
     */
    const string& brief() const noexcept {
        return m_brief;
    }

    /**
     * 设置简要描述
     */
    void brief(const string& brief) {
        m_brief = brief;
    }

    /**
     * 获取详细描述
     */
    const string& details() const noexcept {
        return m_details;
    }

    /**
     * 设置详细描述
     */
    void details(const string& details) {
        m_details = details;
    }

    //------------------------
    // 其他接口
    //------------------------

    /**
     * 计算哈希值
     */
    virtual uint64_t hash() const noexcept;

    /**
     * 转换为字符串表示
     */
    string str() const;

    virtual void save();

    virtual void remove();

    virtual void load();

    virtual IndicatorList getValues(const StockList& stocks, const KQuery& query);
    virtual void saveValues(const StockList& stocks, const KQuery& query, bool replace);

protected:
    string m_name;               ///< 因子名称
    string m_ktype;              ///< K线类型
    string m_brief;              ///< 简要描述
    string m_details;            ///< 详细描述
    Datetime m_create_at;        ///< 创建时间
    Datetime m_update_at;        ///< 更新时间
    Indicator m_formula;         ///< 计算公式指标
    bool m_need_persist{false};  ///< 是否需要持久化
};

typedef shared_ptr<FactorImp> FactorImpPtr;

}  // namespace hku
