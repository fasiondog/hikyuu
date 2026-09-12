/*
 * AllocateFundsBase.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  组合级资金分配（AF）基类：仅聚合形态（MultiSystem）持有。
 *
 *  AF 由三个可替换的算法部件构成，各自对应一个虚函数（L1/L2/L3）：
 *   - L1 `_allocate`  子系统上下文 → 权重（模式 A 名义权重 / 模式 B 真实额度）
 *   - L2 `_toTargets` 权重         → 父账户可执行数量（子系统空间 → 个股空间的折叠点）
 *   - L3 `_checkRisk` 组合风控裁剪（集中度等）
 *
 *  整体入口为 `allocate()`，固定按 L1 → L2 → L3 顺序执行。
 *  单证券形态使用 MoneyManagerBase（MM），两者不再共享类层次、不再共享参数族、不再共享模式。
 */

#pragma once
#ifndef ALLOCATEFUNDSBASE_H_
#define ALLOCATEFUNDSBASE_H_

#include <unordered_map>
#include <vector>

#include "../../utilities/Parameter.h"
#include "../system/SystemPart.h"
#include "../system/TradeSuggestion.h"
#include "../system/SubSystemContext.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

class System;  // 前向声明，避免与 System.h 形成包含环
using SYSPtr = std::shared_ptr<System>;

class AllocateFundsBase;
typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;

/**
 * 客户程序都应使用该指针类型
 * @ingroup AllocateFunds
 */
using AFPtr = AllocateFundsPtr;

/**
 * 组合级资金分配（AF）基类。内含 L1/L2/L3 三个可替换算法部件。
 * @note 仅聚合形态（MultiSystem）使用；单证券形态不会调用本类。
 * @ingroup AllocateFunds
 */
class HKU_API AllocateFundsBase : public enable_shared_from_this<AllocateFundsBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    AllocateFundsBase();
    explicit AllocateFundsBase(const string& name);
    AllocateFundsBase(const AllocateFundsBase&) = default;
    virtual ~AllocateFundsBase();

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

    /** 克隆操作 */
    AllocateFundsPtr clone();

    /** 设定交易账户（父/聚合系统的真实账户） */
    void setTM(const TradeManagerPtr& tm) {
        m_tm = tm;
    }

    /** 获取交易账户 */
    TradeManagerPtr getTM() const {
        return m_tm;
    }

    /** 设置查询条件 */
    void setQuery(const KQuery& query) {
        m_query = query;
    }

    /** 获取查询条件 */
    const KQuery& getQuery() const {
        return m_query;
    }

    /** 设置分配模式：A（信号汇总，默认）或 B（资金划拨 / FOF-MOM）。
     *  @note 本字段是分配模式的**唯一来源**，聚合系统不再自行保存一份。 */
    void setMode(const string& mode) {
        m_mode = (mode == "B" || mode == "b") ? "B" : "A";
    }

    /** 获取分配模式 */
    const string& getMode() const {
        return m_mode;
    }

    /** 权重表：子系统 → 权重（L1 输出、L2 输入） */
    using Weights = std::unordered_map<SYSPtr, double>;

    /**
     * L1/L2/L3 统一入口，供聚合 System（MultiSystem）调用。
     * 流程：L1 系统级分配（名义权重 / 真实额度） → L2 行为级换算（模式 A 按比重 / 模式 B 透传）
     *       → L3 组合风控裁剪。
     * @param date 交易日期
     * @param tm 父（聚合）系统的真实交易账户
     * @param suggestions [in/out] 各子系统上送的建议，L2 会就地改写为父账户可执行数量
     * @param contexts 各子系统上下文（虚拟账户资金 / 模式 B 额度等），供 L1 分配
     * @param query 查询条件（K 线类型等）
     */
    void allocate(const Datetime& date, const TradeManagerPtr& tm, TradeSuggestionList& suggestions,
                  SubSystemContextList& contexts, const KQuery& query);

    /** L1 系统级分配：模式 A 返回名义权重（suggested weight），模式 B 返回真实额度（写入
     *  contexts[i].quota）。
     *  @note 默认返回等权（1/N）；参数 weight-list 非空时改用固定权重（迁移 AF_FixedWeight /
     *        AF_FixedWeightList）。 */
    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query);

    /** 解析参数 weight-list（逗号分隔的固定权重）为归一化权重向量；为空、数量与子系统不符或总和<=0
     *  时返回空向量（调用方回退等权）。迁移自 AF_FixedWeight / AF_FixedWeightList。 */
    std::vector<double> _parseWeightList(size_t expect_n) const;

    /** L1 骨架：按「子系统下标 → 权重」填充 L1 结果，并在模式 B 下写入 contexts[i].quota
     *  （fixed-amount>0 时优先按固定额度，否则 权重 × 父总资产）。
     *  @note **不做归一化**，权重语义完全由子类决定；缺省下标（weights 短于 contexts）回退等权。 */
    Weights _applyWeights(const Datetime& date, const TradeManagerPtr& tm,
                          SubSystemContextList& contexts, const KQuery& query,
                          const std::vector<double>& weights) const;

    /** L2 行为级换算：模式 A 按 assets_ratio / 权重换算为父账户数量；模式 B 透传子系统 number。
     *  @note 默认实现为模式 A（等权到仓）：将每条建议的 number 改写为「权重 × 父总资产 / 计划价」的目标股数，
     *        SELL/CLEAR 标记全平（MAX_DOUBLE）。 */
    virtual void _toTargets(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const Weights& sys_weight,
                            const KQuery& query);

    /** L3 组合风控裁剪：模式 A 做组合维度风控（集中度 / 换手率等）；模式 B 可关闭或仅做总量校验。
     *  @note 默认按参数 max-single-position 限制单标的集中度。 */
    virtual void _checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const KQuery& query);

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual AllocateFundsPtr _clone() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    string m_mode{"A"};  // 分配模式：A=信号汇总（默认） / B=资金划拨（FOF-MOM）
    KQuery m_query;
    TradeManagerPtr m_tm;
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
BOOST_SERIALIZATION_ASSUME_ABSTRACT(AllocateFundsBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @ingroup AllocateFunds
 */
#define ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                            \
    friend class boost::serialization::access;                      \
    template <class Archive>                                        \
    void serialize(Archive& ar, const unsigned int version) {       \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateFundsBase); \
    }
#else
#define ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define ALLOCATE_FUNDS_IMP(classname)                \
public:                                              \
    virtual AllocateFundsPtr _clone() override {     \
        return std::make_shared<classname>();        \
    }

HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsBase&);
HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::AllocateFundsBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::AllocateFundsPtr> : ostream_formatter {};
#endif

#endif /* ALLOCATEFUNDSBASE_H_ */
