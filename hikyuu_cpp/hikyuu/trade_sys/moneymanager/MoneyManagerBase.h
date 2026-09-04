/*
 * MoneyManagerBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef MONEYMANAGERBASE_H_
#define MONEYMANAGERBASE_H_

#include "../../utilities/Parameter.h"
#include "../system/SystemPart.h"
#include "../system/TradeSuggestion.h"
#include "../system/SubSystemContext.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

class System;  // 前向声明，避免与 System.h 形成包含环
using SYSPtr = std::shared_ptr<System>;

/**
 * 资金管理基类
 * @ingroup MoneyManager
 */
class HKU_API MoneyManagerBase : public enable_shared_from_this<MoneyManagerBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    MoneyManagerBase();
    explicit MoneyManagerBase(const string& name);
    MoneyManagerBase(const MoneyManagerBase&) = default;
    virtual ~MoneyManagerBase();

    /** 获取名称 */
    const string& name() const {
        return m_name;
    }

    /** 设置名称 */
    void name(const string& name) {
        m_name = name;
    }

    /** 复位 */
    void reset();

    /**
     * 设定交易账户
     * @param tm 指定的交易账户
     */
    void setTM(const TradeManagerPtr& tm) {
        m_tm = tm;
    }

    /**
     * 获取交易账户
     * @return
     */
    TradeManagerPtr getTM() const {
        return m_tm;
    }

    /** 设置查询条件 */
    void setQuery(const KQuery& query) {
        m_query = query;
    }

    /** 获取交易的K线类型 */
    const KQuery& getQuery() const {
        return m_query;
    }

    typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
    /** 克隆操作 */
    MoneyManagerPtr clone();

    /** 接收实际交易变化情况 */
    void buyNotify(const TradeRecord& tr);

    /** 子类接收实际交易变化情况接口，一般存在多次增减仓的情况才需要重载 */
    virtual void _buyNotify(const TradeRecord&) {}

    /** 接收实际交易变化情况 */
    void sellNotify(const TradeRecord& tr);

    /** 子类接收实际交易变化情况接口，一般存在多次增减仓的情况才需要重载 */
    virtual void _sellNotify(const TradeRecord&) {}

    /**
     * 获取指定交易对象可卖出的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param risk 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
     * @param from 信号来源
     * @note 默认实现返回 MAX_DOUBLE 卖出全部; 多次减仓才需要实现该接口
     */
    double getSellNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                         SystemPart from);

    /**
     * 获取指定交易对象可卖空的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param from 信号来源
     * @param risk 承担的交易风险，如果为Null<price_t>，表示不设损失上限
     */
    double getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                              price_t risk, SystemPart from);

    /**
     * 获取指定交易对象空头回补的买入数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param from 信号来源
     * @param risk 承担的交易风险，如果为Null<price_t>，表示不设损失上限
     */
    double getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                             price_t risk, SystemPart from);

    /**
     * 获取指定交易对象可买入的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param from 信号来源
     * @param risk 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
     */
    double getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                        SystemPart from);

    /** 当前买入交易次数, 连续买入计数，一旦接收卖出将恢复置0 */
    size_t currentBuyCount(const Stock&) const;

    /** 当前卖出交易次数，连续卖出计数，一旦接收买入将恢复置0 */
    size_t currentSellCount(const Stock&) const;

    /**
     * 组合级资金分配（MM L1/L2/L3 统一入口），供聚合 System（MultiSystem）调用。
     * 流程：L1 系统级分配（名义权重 / 真实额度） → L2 行为级换算（模式 A 按比重 / 模式 B 透传）
     *        → L3 组合风控裁剪。
     * @note 单证券形态不会调用本接口；本接口是「功能等价 PF」的核心分配引擎。
     * @param date 交易日期
     * @param tm 父（聚合）系统的真实交易账户
     * @param suggestions [in/out] 各子系统上送的建议，L2 会就地改写为父账户可执行数量
     * @param contexts 各子系统上下文（虚拟账户资金 / 模式 B 额度等），供 L1 分配
     * @param query 查询条件（K 线类型等）
     */
    /** 设置分配模式：A（信号汇总，默认）或 B（资金划拨 / FOF-MOM） */
    void setMode(const string& mode) {
        m_mode = (mode == "B" || mode == "b") ? "B" : "A";
    }

    /** 获取分配模式 */
    const string& getMode() const {
        return m_mode;
    }

    void allocate(const Datetime& date, const TradeManagerPtr& tm, TradeSuggestionList& suggestions,
                  SubSystemContextList& contexts, const KQuery& query);

    /** L1 系统级分配：模式 A 返回名义权重（suggested weight），模式 B 返回真实额度（写入 contexts[i].quota）。
     *  @note 默认返回等权（1/N）；参数 weight-list 非空时改用固定权重（迁移 AF_FixedWeight/FixedWeightList）。
     *        保证任意 MM 在聚合形态下零改动即可工作。 */
    virtual std::unordered_map<SYSPtr, double> _allocateSystemWeight(const Datetime& date,
                                                                      const TradeManagerPtr& tm,
                                                                      SubSystemContextList& contexts,
                                                                      const KQuery& query);

    /** 解析参数 weight-list（逗号分隔的固定权重）为归一化权重向量；为空、数量与子系统不符或总和<=0 时
     *  返回空向量（调用方回退等权）。迁移自 AF_FixedWeight / AF_FixedWeightList。 */
    std::vector<double> _parseWeightList(size_t expect_n) const;

    /** L2 行为级换算：模式 A 按 assets_ratio / 权重换算为父账户数量；模式 B 透传子系统 number。
     *  @note 默认实现为模式 A（等权到仓）：将每条建议的 number 改写为「权重 × 父总资产 / 计划价」的目标股数，
     *        SELL/CLEAR 标记全平（MAX_DOUBLE）。 */
    virtual void _allocateSuggestions(const Datetime& date, const TradeManagerPtr& tm,
                                      TradeSuggestionList& suggestions,
                                      const std::unordered_map<SYSPtr, double>& sys_weight,
                                      const KQuery& query);

    /** L3 组合风控裁剪：模式 A 做组合维度风控（集中度 / 换手率等）；模式 B 可关闭或仅做总量校验。
     *  @note 默认空实现（不裁剪）。 */
    virtual void _checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const KQuery& query);

    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                 price_t risk, SystemPart from) = 0;

    virtual double _getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                  price_t risk, SystemPart from);

    virtual double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from);

    virtual double _getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from);

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual MoneyManagerPtr _clone() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    string m_mode{"A"};  // 分配模式：A=信号汇总（默认） / B=资金划拨（FOF-MOM）
    KQuery m_query;
    TradeManagerPtr m_tm;
    unordered_map<Stock, std::pair<size_t, size_t>> m_buy_sell_counts;
    bool m_is_python_object{false};

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_mode);
        // m_query、m_tm都是系统运行时临时设置，不需要序列化
        // ar & BOOST_SERIALIZATION_NVP(m_query);
        // ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_mode);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MoneyManagerBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public MoneyManagerBase {
 *     MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MoneyManager
 */
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                           \
    friend class boost::serialization::access;                     \
    template <class Archive>                                       \
    void serialize(Archive& ar, const unsigned int version) {      \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase); \
    }
#else
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * 客户程序都应使用该指针类型
 * @ingroup MoneyManager
 */
typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
typedef shared_ptr<MoneyManagerBase> MMPtr;

#define MONEY_MANAGER_IMP(classname)                                                          \
public:                                                                                       \
    virtual MoneyManagerPtr _clone() override {                                               \
        return std::make_shared<classname>();                                                 \
    }                                                                                         \
    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, \
                                 price_t risk, SystemPart from) override;

HKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MoneyManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::MoneyManagerPtr> : ostream_formatter {};
#endif

#endif /* MONEYMANAGERBASE_H_ */
