/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 *
 *  递归组合重构：聚合交易系统（组合回测）
 *  持有多个子系统（单证券或嵌套聚合），在开盘/收盘阶段分别驱动并汇总下单。
 *  阶段 3：双模式（A/B）+ 任意嵌套 + MM L1/L2/L3 + 调仓周期 + 层级路径。
 *  模式 A（默认）：父给子「影子账户」，父按权重统一分配并下单（功能等价 PF 信号汇总）。
 */

#pragma once
#include "../System.h"
#include "../../../trade_manage/crt/crtTM.h"
#include "../../moneymanager/crt/MM_Nothing.h"

namespace hku {

class SelectorBase;  // 前向声明，避免与 SelectorBase.h（内含 System.h）形成包含环

class HKU_API MultiSystem : public System {
public:
    MultiSystem() : System() {
        // 聚合形态父系统的默认 MM 为模式 A（信号汇总，基类 allocate 实现等权分配）
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const string& name) : System(name) {
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const SystemPtr& sys) : m_sys_list(sys ? SystemList{sys} : SystemList{}) {
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const SystemList& sys_list, const string& name = "MultiSystem")
    : System(name), m_sys_list(sys_list) {
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    virtual ~MultiSystem() = default;

    /** 添加子系统（含循环引用检测，拒绝包含自身或已存在的节点） */
    void add(const SystemPtr& sys);

    /** 获取子系统列表 */
    const SystemList& getSystemList() const {
        return m_sys_list;
    }

    virtual bool isComposite() const override {
        return true;
    }
    virtual const SystemList& getSubSystemList() const override {
        return m_sys_list;
    }

    virtual void run(const KData& kdata, bool reset = true, bool resetAll = false) override;
    virtual MomentResult runMoment(const Datetime& datetime) override;
    virtual MomentResult runMomentOnOpen(const Datetime& datetime) override;
    virtual MomentResult runMomentOnClose(const Datetime& datetime) override;

    virtual void readyForRun() override;
    virtual void _reset() override;
    virtual void _forceResetAll() override;
    virtual SystemPtr _clone() override;

    /** 层级路径 */
    virtual const string& getPath() const override {
        return m_path;
    }

    /** 设置运行模式：A（信号汇总，默认）/ B（资金划拨 / FOF-MOM）。
     *  模式 B 下父在调仓日通过 L1 产出真实额度并回写各子系统。 */
    void setMode(const string& mode) {
        m_mode = (mode == "B" || mode == "b") ? "B" : "A";
        if (getMM()) {
            getMM()->setMode(m_mode);
        }
    }

    /** 获取运行模式 */
    const string& getMode() const {
        return m_mode;
    }

    /** 设置子系统影子账户初始资金（模式 A 固定值 / 模式 B 初始额度） */
    void setSubInitCash(price_t cash) {
        m_sub_init_cash = cash > 0.0 ? cash : m_sub_init_cash;
    }

    /** 获取子系统影子账户初始资金 */
    price_t getSubInitCash() const {
        return m_sub_init_cash;
    }

    /** 设置调仓周期（天）；<=1 表示每个收盘日都再平衡 */
    void setAdjustCycle(int cycle) {
        m_adjust_cycle = cycle > 0 ? cycle : 1;
    }

    /** 获取调仓周期（天） */
    int getAdjustCycle() const {
        return m_adjust_cycle;
    }

    /** 设置是否在收盘阶段执行调仓下单 */
    void setTradeOnClose(bool v) {
        m_trade_on_close = v;
    }

    /** 获取是否在收盘阶段执行调仓下单 */
    bool getTradeOnClose() const {
        return m_trade_on_close;
    }

    /** 设置交易对象选择器（可选；设置后仅运行 SE 选中的子系统，未选中可清仓） */
    void setSE(const std::shared_ptr<SelectorBase>& se) {
        m_se = se;
    }

    /** 获取交易对象选择器 */
    const std::shared_ptr<SelectorBase>& getSE() const {
        return m_se;
    }

    /** 设置未选中子系统是否强制清仓（sell_at_not_selected） */
    void setSellAtNotSelected(bool v) {
        m_sell_at_not_selected = v;
    }

    /** 获取未选中子系统是否强制清仓 */
    bool getSellAtNotSelected() const {
        return m_sell_at_not_selected;
    }

    /** 获取各调仓日的换手率（成交金额 / 调仓前总资产） */
    const std::vector<std::pair<Datetime, double>>& getAdjustTurnover() const {
        return m_adjust_turnover;
    }

    /** 模式 B 额度回写（写入子系统虚拟账户，供下期运行；聚合子系统自动穿透） */
    virtual void setSubSystemQuota(const SYSPtr& sub_sys, const Datetime& date,
                                   price_t quota) override;

    /** 将本时刻直接子系统的成交转译为对上建议（嵌套能力粘合剂） */
    virtual TradeSuggestionList toSuggestions() const override {
        return m_last_suggestions;
    }

public:
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) override;
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) override;
    virtual void clearDelayBuyRequest() override;
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date) override;

private:
    SystemList m_sys_list;
    string m_path;                 // 层级路径，如 I/D/A
    size_t m_close_day_index{0};    // 收盘日计数，用于调仓周期判定
    price_t m_sub_init_cash{100000.0};  // 子系统影子账户初始资金（模式 A）
    int m_adjust_cycle{1};          // 调仓周期（天）；<=1 表示每个收盘日都再平衡
    bool m_trade_on_close{true};    // 是否在收盘阶段执行调仓下单
    string m_mode{"A"};             // 运行模式：A=信号汇总（默认）/ B=资金划拨（FOF-MOM）
    std::shared_ptr<SelectorBase> m_se;  // 交易对象选择器（可选）
    bool m_sell_at_not_selected{true};   // 未选中子系统是否强制清仓（仅在设置 SE 后生效）
    std::vector<std::pair<Datetime, double>> m_adjust_turnover;  // 调仓日换手率（成交额/调仓前总资产）
    TradeSuggestionList m_last_suggestions;  // 最近一次收盘产生的对上建议
    std::vector<TradeRecordList> m_open_trades;  // 当日各子系统开盘成交（延迟请求兑现），供收盘汇总
    Datetime m_open_trades_date;  // m_open_trades 所属交易日；收盘阶段据此防越界与跨日残留（运行时态，不序列化）

    // 检查 candidate 子树（含自身）是否包含 target（用于循环引用检测）
    static bool _subtreeContains(const SystemPtr& candidate, System* target);

    /** 将一组成交按标的聚合为净建议（标注来源子系统 sys） */
    TradeSuggestionList _toSuggestions(const SystemPtr& sys,
                                       const TradeRecordList& trades) const;

    /** 判定给定日期是否为调仓日（仅调仓日执行再平衡） */
    bool _isAdjustDate() const;

    /** 在父真实账户执行已换算的建议（先卖后买） */
    void _executeSuggestions(const Datetime& date, const TradeSuggestionList& suggestions,
                             KQuery::KType ktype, TradeRecordList& out_trades);

    /** 收盘阶段：驱动各子系统生成信号，合并「开盘+收盘」成交转译为对上建议，
     *  MM 分配后由父统一下单；返回父实际成交。供 runMoment / runMomentOnClose 复用。 */
    TradeRecordList _closePhase(const Datetime& datetime);

    /** 获取指定标的在指定日期的收盘价（用于未选中子系统清仓建议定价）；无数据返回 0 */
    price_t _getClosePrice(const Datetime& date, const Stock& stock) const;

    /** 开盘阶段强制卖出已退市标的的父持仓（退市 = 标的最后交易日早于当前运行日期） */
    TradeRecordList _forceSellDelisted(const Datetime& date);

//========================================
// 序列化支持
//========================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(System);
        ar& BOOST_SERIALIZATION_NVP(m_sys_list);
        ar& BOOST_SERIALIZATION_NVP(m_path);
        ar& BOOST_SERIALIZATION_NVP(m_sub_init_cash);
        ar& BOOST_SERIALIZATION_NVP(m_adjust_cycle);
        ar& BOOST_SERIALIZATION_NVP(m_trade_on_close);
        ar& BOOST_SERIALIZATION_NVP(m_sell_at_not_selected);
        ar& BOOST_SERIALIZATION_NVP(m_mode);
        ar& BOOST_SERIALIZATION_NVP(m_se);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

}  // namespace hku
