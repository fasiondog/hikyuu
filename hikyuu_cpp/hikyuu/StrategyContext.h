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
 * Strategy context, it defines the security / K-line level information contained when the strategy
 * is executed
 * @ingroup Strategy
 *
 */
class HKU_API StrategyContext {
public:
    StrategyContext() = default;
    virtual ~StrategyContext() = default;

    /**
     * Constructor
     * @param stockCodeList the given security code list, e.g. {"sz000001", "sz000002"}
     */
    explicit StrategyContext(const vector<string>& stockCodeList);

    /**
     * Constructor
     * @note If the security list contains ("ALL") it means all the securities;
     *       1. The given K-line type list also determines the priority loading order of the K-line
     *          data, the earlier ones are loaded first.
     *       2. When ktypelist or preloadNum is not given, the global config file parameters are
     *          used
     * @param stockCodeList the given security code list, e.g. {"sh000001", "sz000001"}
     * @param ktypeList the given K-line data list, e.g. {"day", "min"}
     * @param preloadNum the given preload number, e.g. {{"min_max", 100}, {"day_max", 200}}
     */
    StrategyContext(const vector<string>& stockCodeList, const vector<KQuery::KType>& ktypeList,
                    const unordered_map<string, int64_t>& preloadNum = {});

    // A user-defined move constructor and assignment cause python to fail to exit normally
    // StrategyContext(const StrategyContext&) = default;
    // StrategyContext(StrategyContext&& rv) = delete;
    // StrategyContext& operator=(const StrategyContext&) = default;
    // StrategyContext& operator=(StrategyContext&&) = delete;

    /**
     * Whether all the securities are loaded; as long as stockCodeList contains "ALL"(case
     * insensitive) it is regarded as loading everything
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

    void setPreloadNum(const unordered_map<string, int64_t>& preloadNum);

    const unordered_map<string, int64_t>& getPreloadNum() const noexcept {
        return m_preloadNum;
    }

    /**
     * The implicit security list that must be loaded by default
     * @note It affects the trading calendar judgment and some securities that are often used as the
     *       default comparison benchmark, and is usually used as the default value of some
     *       functions
     */
    const vector<string>& getMustLoadStockCodeList() const noexcept {
        return m_mustLoad;
    }

    /**
     * Return all the security code lists that need to be loaded (including the given security list
     * and the default must-load security list)
     * @return vector<string>
     */
    vector<string> getAllNeedLoadStockCodeList() const noexcept;

    string str() const;

private:
    void _removeDuplicateCode(const vector<string>& stockCodeList);
    void _checkAndRemoveDuplicateKType(const vector<KQuery::KType>& ktypeList);

private:
    Datetime m_startDatetime{19901219};
    vector<string> m_mustLoad{"sh000001", "sh000300"};  // The stock that must be loaded by default
    vector<string> m_stockCodeList;
    vector<KQuery::KType> m_ktypeList;
    unordered_map<string, int64_t> m_preloadNum;
};

HKU_API std::ostream& operator<<(std::ostream& os, const StrategyContext& context);

}  // namespace hku