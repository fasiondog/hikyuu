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
#include <set>
#include "../System.h"
#include "../../../trade_manage/crt/crtTM.h"
#include "../../moneymanager/crt/MM_Nothing.h"
#include "../../allocatefunds/crt/AF_EqualWeight.h"

namespace hku {

class SelectorBase;  // 前向声明，避免与 SelectorBase.h（内含 System.h）形成包含环

class HKU_API MultiSystem : public System {
public:
    MultiSystem() : System() {
        _initAxisParam();
        // 聚合形态父系统的默认 MM 为模式 A（信号汇总，基类 allocate 实现等权分配）
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const string& name) : System(name) {
        _initAxisParam();
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const SystemPtr& sys) : m_sys_list(sys ? SystemList{sys} : SystemList{}) {
        _initAxisParam();
        if (!getMM()) {
            setMM(MM_Nothing());
        }
    }
    explicit MultiSystem(const SystemList& sys_list, const string& name = "MultiSystem")
    : System(name), m_sys_list(sys_list) {
        _initAxisParam();
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

    /** 设置资金分配实例（组合级资金分配 AF，承载 L1/L2/L3 三个算法部件） */
    void setAF(const AllocateFundsPtr& af) {
        if (af) {
            m_af = af;
        }
    }

    /** 获取资金分配实例 */
    const AllocateFundsPtr& getAF() const {
        return m_af;
    }

    /** 设置运行模式：A（信号汇总，默认）/ B（资金划拨 / FOF-MOM）。
     *  模式 B 下父在调仓日通过 L1 产出真实额度并回写各子系统。
     *  @note 模式由 AF 持有（唯一来源），本方法直接写入当前 AF。 */
    void setMode(const string& mode) {
        if (m_af) {
            m_af->setMode(mode);
        }
    }

    /** 获取运行模式（来自 AF） */
    const string& getMode() const;

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

    /** 设置驱动时间轴模式（参数 axis-mode）：
     *  - "kdata"（默认）：以 run(kdata) 入参 KData 自带日期序列为驱动轴（既有行为）
     *  - "calendar"：以 setDateAxis() 注入的固定日期表（如全市场交易日历）为驱动轴；
     *               入参 KData 退化为 query/ktype 与价格查询上下文，其自带日期不再驱动。
     *  非法取值告警并回退 "kdata"。 */
    void setAxisMode(const string& mode);

    /** 获取驱动时间轴模式 */
    string getAxisMode() const {
        return tryGetParam<string>("axis-mode", "kdata");
    }

    /** 设置调仓模式（承接 master PF 的 adjust_mode，见 design.md §4.3）：
     *  - "query" / "day"（默认）：沿用 m_adjust_cycle 的「每 N 个收盘日」计数判定；
     *  - "week" / "month" / "quarter" / "year"：在驱动轴上按「周期内第 adjust_cycle 日」展开调仓日表；
     *  非法取值告警并回退 "query"。 */
    void setAdjustMode(const string& mode);

    /** 获取调仓模式 */
    string getAdjustMode() const {
        return tryGetParam<string>("adjust-mode", "query");
    }

    /** 设置调仓日非交易日时是否顺延至当周期内首个交易日（仅 week/month/quarter/year 展开时生效） */
    void setDelayToTradingDay(bool v) {
        setParam<bool>("delay-to-trading-day", v);
    }

    /** 获取是否顺延至交易日 */
    bool getDelayToTradingDay() const {
        return tryGetParam<bool>("delay-to-trading-day", true);
    }

    /** 按 master Portfolio 算法，在给定交易日轴上计算调仓日集合（纯函数，便于单测与外部预览）。
     *  @param dates 已排序的交易日轴（通常是驱动轴；停牌/非交易日不应出现在轴上）
     *  @param mode "week" | "month" | "quarter" | "year"（其余取值如 query/day/非法值返回空）
     *  @param adjust_cycle 周期内第 N 日（<=0 视为 1）；week 模式下为 dayOfWeek（0=周日,1=周一…6=周六）
     *  @param delay_to_trading_day true 时顺延至当周期内首个交易日；false 时仅在恰为第 N 日命中
     *  @return 升序去重的调仓日列表
     *  @note 与 master Portfolio::_calculateAdjustDate* 行为对齐（见 design.md §4.3） */
    static DatetimeList calcAdjustDates(const DatetimeList& dates, const string& mode,
                                        int adjust_cycle, bool delay_to_trading_day);

    /** 设置固定时间轴（仅 axis-mode == "calendar" 时作为驱动轴；空轴时回退 kdata 轴并告警） */
    void setDateAxis(const DatetimeList& dates) {
        m_date_axis = dates;
    }

    /** 获取固定时间轴 */
    const DatetimeList& getDateAxis() const {
        return m_date_axis;
    }

    /** 清空固定时间轴（清空后 calendar 模式回退为 kdata 轴） */
    void clearDateAxis() {
        m_date_axis.clear();
    }

    /** 设置外部调仓日表（非空时优先作为调仓日判据，用于映射 master 的
     *  adjust_mode = "week"/"month"/"quarter"/"year" 与 delay_to_trading_day；
     *  传入日期统一归一化为当日零点后存入，只有命中表内日期才视为调仓日）。 */
    void setAdjustDates(const DatetimeList& dates);

    /** 获取外部调仓日表（已归一化为当日零点） */
    const std::set<Datetime>& getAdjustDates() const {
        return m_adjust_dates;
    }

    /** 清空外部调仓日表（清空后回退 m_adjust_cycle 的收盘日计数判定） */
    void clearAdjustDates() {
        m_adjust_dates.clear();
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
    AllocateFundsPtr m_af{AF_EqualWeight()};  // 组合级资金分配（AF，含 L1/L2/L3）；运行模式由其持有
    std::shared_ptr<SelectorBase> m_se;  // 交易对象选择器（可选）
    bool m_sell_at_not_selected{true};   // 未选中子系统是否强制清仓（仅在设置 SE 后生效）
    std::vector<std::pair<Datetime, double>> m_adjust_turnover;  // 调仓日换手率（成交额/调仓前总资产）
    TradeSuggestionList m_last_suggestions;  // 最近一次收盘产生的对上建议
    std::vector<TradeRecordList> m_open_trades;  // 当日各子系统开盘成交（延迟请求兑现），供收盘汇总
    std::vector<FundsRecord> m_sub_funds_before;  // 当日各子系统「交易前」资金快照，供 _toSuggestions 计算三比重（运行时态，不序列化）
    Datetime m_open_trades_date;  // m_open_trades/m_sub_funds_before 所属交易日；收盘阶段据此防越界与跨日残留（运行时态，不序列化）
    DatetimeList m_date_axis;     // 固定时间轴：axis-mode="calendar" 时的驱动日期表（运行时态，不序列化）
    std::set<Datetime> m_adjust_dates;  // 外部调仓日表（归一化至当日零点；非空时优先于 m_adjust_cycle，运行时态，不序列化）
    std::set<Datetime> m_auto_adjust_dates;  // adjust-mode 自动展开的调仓日表（运行时态，不序列化，不覆盖外部注入）

    /** 注册聚合系统自身参数（axis-mode / adjust-mode / delay-to-trading-day） */
    void _initAxisParam() {
        setParam<string>("axis-mode", "kdata");
        // v5：承接 master PF 的 adjust_mode / delay_to_trading_day（见 docs/design/pf_af_compat/design.md §4.3）
        setParam<string>("adjust-mode", "query");
        setParam<bool>("delay-to-trading-day", true);
    }

    // 检查 candidate 子树（含自身）是否包含 target（用于循环引用检测）
    static bool _subtreeContains(const SystemPtr& candidate, System* target);

    /** 将一组成交按标的聚合为净建议（标注来源子系统 sys）。
     *  funds_before 为子系统当日「交易前」资金快照，用于计算建议的三比重（cash/assets/target_position）。 */
    TradeSuggestionList _toSuggestions(const SystemPtr& sys, const TradeRecordList& trades,
                                       const FundsRecord& funds_before) const;

    /** 判定给定日期是否为调仓日（仅调仓日执行再平衡）：
     *  外部调仓日表优先，其次 adjust-mode 自动展开表，最后回退 m_adjust_cycle 的收盘日计数 */
    bool _isAdjustDate(const Datetime& date) const;

    /** v5：把 adjust-mode ∈ {week,month,quarter,year} 内化为调仓日表（design.md §4.3）。
     *  仅在外部 setAdjustDates() 未注入（m_adjust_dates 为空）时生效，结果写入 m_auto_adjust_dates。 */
    void _expandAdjustDates(const DatetimeList& axis);

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
        if (version < 1) {
            // v5 兼容：旧档案此位置为 m_mode（运行模式已迁移至 AllocateFundsBase），读出后回灌 AF。
            string legacy_mode = "A";
            ar& boost::serialization::make_nvp("m_mode", legacy_mode);
            if (m_af) {
                m_af->setMode(legacy_mode);
            }
        } else {
            // v5：资金分配实例（含 L1/L2/L3 与运行模式）随聚合系统一起序列化
            ar& BOOST_SERIALIZATION_NVP(m_af);
        }
        ar& BOOST_SERIALIZATION_NVP(m_se);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef shared_ptr<MultiSystem> MultiSystemPtr;

/**
 * master 兼容别名：master 中 PF_Simple / PF_WithoutAF 返回 PortfolioPtr。
 * feature/next 中 PF 即 MultiSystem 的具体实现（见 docs/design/pf_af_compat/design.md §4.5），
 * 保留该别名以便存量 `PortfolioPtr pf = PF_Simple(...)` 继续编译（不再支持 Portfolio 类方法）。
 * @ingroup Portfolio
 */
using PortfolioPtr = MultiSystemPtr;

}  // namespace hku

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_VERSION(::hku::MultiSystem, 1)
#endif
