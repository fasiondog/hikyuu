/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#pragma once

#include "DataType.h"
#include "KQuery.h"

namespace hku {

/**
 * 策略上下文，定义策略执行时包含的证券/K线级别信息
 * @ingroup Strategy
 *
 */
class HKU_API StrategyContext {
public:
    StrategyContext() = default;
    virtual ~StrategyContext() = default;

    /**
     * 构造函数
     * @note 未指定 ktypelist 时，默认按预加载参数加载全部
     * @param stockCodeList 指定的证券代码列表，如：如：{"sz000001", "sz000002"}
     */
    explicit StrategyContext(const vector<string>& stockCodeList);

    /**
     * 构造函数
     * @note 证券列表中如果包含 ("ALL") 则表示全部证券;
     * 指定K线类型列表同时影响着K线数据的优先加载顺序，靠前的将优先加载。同时受系统配置中预加载参数影响！
     * @param stockCodeList 指定的证券代码列表，如：{"sh000001", "sz000001"}
     * @param ktypeList 指定的 K线数据列表，如：{"day", "min"}
     */
    StrategyContext(const vector<string>& stockCodeList, const vector<KQuery::KType>& ktypeList);

    // 自定义移动构造与赋值会引起 python 中无法正常退出
    // StrategyContext(const StrategyContext&) = default;
    // StrategyContext(StrategyContext&& rv) = delete;
    // StrategyContext& operator=(const StrategyContext&) = default;
    // StrategyContext& operator=(StrategyContext&&) = delete;

    /**
     * 是否为加载全部证券，只要 stockCodeList 包含 "ALL"(不区分大小写) ，即认为加载全部
     * @return true
     * @return false
     */
    bool isAll() const noexcept;

    bool empty() const noexcept {
        return m_stockCodeList.empty();
    }

    Datetime startDatetime() const noexcept {
        return m_startDatetime;
    }

    void setStockCodeList(const vector<string>& stockList) {
        _removeDuplicateCode(stockList);
    }

    const vector<string>& getStockCodeList() const noexcept {
        return m_stockCodeList;
    }

    void setKTypeList(const vector<KQuery::KType>& ktypeList) {
        _checkAndRemoveDuplicateKType(ktypeList);
    }

    const vector<KQuery::KType>& getKTypeList() const noexcept {
        return m_ktypeList;
    }

    /**
     * 隐含的默认必须被加载的证券列表
     * @note 影响交易日历判断、和某些常被作为默认比较基准的证券，通常被作为某些函数的默认值
     */
    const vector<string>& getMustLoadStockCodeList() const noexcept {
        return m_mustLoad;
    }

    /**
     * 返回所有需要加载的证券列表（含指定的证券列表和默认包含必须加载的证券列表）
     * @return vector<string>
     */
    vector<string> getAllNeedLoadStockCodeList() const noexcept;

    string str() const;

private:
    void _removeDuplicateCode(const vector<string>& stockCodeList);
    void _checkAndRemoveDuplicateKType(const vector<KQuery::KType>& ktypeList);

private:
    Datetime m_startDatetime{19901219};
    vector<string> m_mustLoad{"sh000001", "sh000300"};  // 默认必须加载的 stock
    vector<string> m_stockCodeList;
    vector<KQuery::KType> m_ktypeList;
};

HKU_API std::ostream& operator<<(std::ostream& os, const StrategyContext& context);

}  // namespace hku