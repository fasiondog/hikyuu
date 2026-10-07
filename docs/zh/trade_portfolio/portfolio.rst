.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

投资组合|PF
=============

在 Hikyuu 中，投资组合以系统交易策略为单位，可以使用不同目标、同一交易策略的组合。

.. note::

   自 ``feature/next`` 起，原独立的 ``Portfolio``（``SimplePortfolio`` / ``WithoutAFPortfolio``）已由
   :class:`MultiSystem` 承接：**PF 本质就是 MultiSystem 的一个预置配置**。
   本页描述的 ``PF_Simple`` / ``PF_WithoutAF`` / ``PF_SignalAggregate`` 是**预置工厂**，其**返回值为**
   :class:`MultiSystem`，不再是 ``Portfolio`` 对象。新项目也可直接使用 :class:`MultiSystem`
   （见 :doc:`../trade_sys/system`）。

   组合的执行范式由 AF 承载的**运行模式**决定，三个工厂对应三种模式：

   .. list-table::
      :header-rows: 1

      * - 工厂
        - 模式
        - 定位
      * - ``PF_Simple``
        - B 资金配置
        - 配额划拨、子自主交易、父透传（FOF/MOM）
      * - ``PF_SignalAggregate``
        - A 信号汇总
        - 子系统为纯信号源，父按 L2 目标换算统一下单
      * - ``PF_WithoutAF``
        - C 共享账户兼容
        - 子系统直接在父真实账户上按自身 MM 交易（旧 Portfolio 行为）

   每个工厂另提供按模式字母命名的薄别名 ``PF_ModeA`` / ``PF_ModeB`` / ``PF_ModeC``（同一实现、
   参数一致、无行为差异），便于按模式检索对照。

   .. warning::

      ``PF_WithoutAF`` 的名字本义即"不经 AF 做整体资金分配"，对应**模式 C**（重构前 ``Portfolio``
      的行为）。重构初期曾短暂把它映射到模式 A，现已改回模式 C：存量策略**无需任何改动**即可复现
      重构前的回测结果。若在重构窗口内按"模式 A"语义使用过 ``PF_WithoutAF``，请改用
      ``PF_SignalAggregate``。

PF 部件说明:

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>部件命名规范</th>
                <th>部件说明</th>
                <th>部件用途</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>MF_Xxx</td>
                <td>多因子合成（时间截面评分板）</td>
                <td>多因子本质是在时间截面上对候选标的进行评分，所以实际需要配合 Selector (策略选择算法) 使用。</td>
            </tr>
            <tr>
                <td>SE_Xxx</td>
                <td>系统选择算法</td>
                <td>实现标的、系统策略的评估和选取算法。</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>资产分配算法</td>
                <td>用于对时间截面上选中的系统进行资产分配。<br>自 ``feature/next`` 起，AF 为独立基类 :class:`AllocateFundsBase`，承载组合级 L1/L2/L3，见 :doc:`allocate_funds`。</td>
            </tr>
        </tbody>
    </table>
    <p></p>


内建投资组合
------------------

.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    创建一个多标的、单系统策略的投资组合（**返回 MultiSystem，运行于模式 B「资金配置」（Fund Allocation）**）。

    模式 B 语义（配额划拨，FOF/MOM 风格）：每个调仓日由 AF 先为选中的子系统分配额度（L1）；
    选中的子系统在驱动**之前**被校准到其额度（回收影子现金 → 清仓未选中 → 减持超配 → 注资低配），
    并以精确额度自主交易；父透传其真实指令（L2 透传），L3 组合风控跳过（尊重子管理人自主性）。
    未选中的子系统在调仓日强制清仓。子系统影子账户从 0 起步并跟随父账户成本函数。

    调仓模式 adjust_mode 说明：
    - "query" 模式，跟随输入参数 query 中的 ktype，此时 adjust_cycle 为以 query 中的 ktype 决定周期间隔；
    - "day" 模式，adjust_cycle 为调仓间隔天数；
    - "week" | "month" | "quarter" | "year" 模式时，adjust_cycle 为对应的每周第N日、每月第n日、每季度第n日、每年第n日，在 delay_to_trading_day 为 false 时如果当日不是交易日将会被跳过调仓；当 delay_to_trading_day 为 true 时，如果当日不是交易日将会顺延至当前周期内的第一个交易日，如指定每月第1日调仓，但当月1日不是交易日，则将顺延至当月的第一个交易日。

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param AllocateFundsBase af: 资金分配算法（AF，承载 L1/L2/L3，见 :doc:`allocate_funds`）
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式 "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :rtype: MultiSystem

.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False, sell_at_not_selected=False])

    创建无资金分配算法的投资组合（**返回 MultiSystem，运行于模式 C「共享账户兼容」（Shared Account
    Compatibility）**），即重构前 ``WithoutAFPortfolio`` 的行为，用于复现重构**前**的回测结果。

    模式 C 语义（共享账户）：不建影子账户，子系统**直接在父真实账户上**交易（``shared_tm``），
    数量由其自身 MM 决定，因此多个子系统会挤占同一份现金（按入池次序竞争）；父只做 SE 准入与
    持续驱动，**不做 L2 换算、不重复下单**；AF 在此仅作模式载体（L1 不分配额度、L3 默认关闭）。

    驱动集由 SE 语义决定（SE 可自由替换）：调仓日被选中者进入运行池；**未被选中即出池**（与是否
    持仓无关）。出池仍持仓者：``sell_at_not_selected=True`` 时父立即强平；为 ``False``（默认，对齐
    旧引擎）时只在出池日补驱动一次，其后转入 ``force_sell`` 池——该池仅在非调仓日且
    ``sell_at_not_selected=True`` 时才继续被驱动，故默认配置下残余持仓不再被盯盘（旧引擎既定行为）。

    调仓模式 adjust_mode 说明同上。

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式 "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :param bool trade_on_close: 交易是否在收盘时进行（同时下发为子系统的 buy_delay/sell_delay 映射）
    :param bool sys_use_self_tm: 仅为签名兼容保留（模式 C 一律共享父真实账户，**忽略并告警**）
    :param bool sell_at_not_selected: 调仓日未选中的标的是否强制卖出，默认 ``False`` 对齐旧引擎
    :rtype: MultiSystem

.. py:function:: PF_SignalAggregate([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sell_at_not_selected=False, sub_init_cash=100000.0])

    创建运行于**模式 A「信号汇总」（Signal Aggregation）**的投资组合（返回 MultiSystem）。

    模式 A 语义（信号汇总）：子系统在信号「影子账户」上作为纯信号源（信号资金每调仓日重置）；
    父通过 AF L2 把建议换算为父账户目标持仓（目标市值 = 权重 × 仓位意图 × 父总资产，按标的聚合后
    对当前持仓取差额再平衡），并应用 L3 组合风控（如 ``max-single-position``）。
    未选中的子系统默认不清仓（``sell_at_not_selected=False``）。

    这是重构初期由 ``PF_WithoutAF`` 承担的预置；``PF_WithoutAF`` 已改回模式 C，模式 A 预置另立此名。

    调仓模式 adjust_mode 说明同上。

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param AllocateFundsBase af: 资金分配算法（AF，承载 L1/L2/L3，默认等权，见 :doc:`allocate_funds`）
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式 "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :param bool trade_on_close: 交易是否在收盘时进行
    :param bool sell_at_not_selected: 调仓日未选中的标的是否强制卖出
    :param float sub_init_cash: 子系统影子账户的信号资金（每调仓日重置）
    :rtype: MultiSystem


与 master 的差异与迁移
----------------------

.. list-table::
    :header-rows: 1

    * - 维度
      - master
      - 当前实现（工厂直通 MultiSystem）
    * - 返回类型
      - ``PortfolioPtr``
      - ``MultiSystemPtr``（兼容别名 ``PortfolioPtr`` 指向 ``MultiSystemPtr``，存量 ``PortfolioPtr pf = PF_Simple(...)`` 可继续编译）
    * - 类 / 方法
      - ``Portfolio`` / ``SimplePortfolio`` / ``WithoutAFPortfolio`` 及其方法（``run`` / ``getRunningDates`` / ``getCycleEndDates`` / ``lastSuggestion`` …）
      - 不再存在；改用 :class:`MultiSystem` 的 ``run`` / ``getAdjustDates`` / ``getAdjustTurnover`` / ``toSuggestions``。
        其中 ``run(query)`` 另提供兼容重载（等价 master ``Portfolio.run(query)``，以市场交易日历为驱动轴），存量 ``pf.run(query)`` 无需改写
    * - 账户层级
      - 真实 TM + 影子 TM + 子系统账户
      - 父真实 TM + 子系统影子账户 ``TM_SUB``（模式 B 从 0 起步、跟随父账户成本函数；模式 A 以 :meth:`MultiSystem.set_sub_init_cash` 为信号资金、每调仓日重置）；**模式 C 例外**：不建影子账户，子系统共享父真实 TM（等价 master ``WithoutAFPortfolio`` 的 ``shared_tm``）
    * - 资金调拨
      - 调仓日 checkout / checkin
      - 模式 B：调仓日**先校准后驱动**（回收/清仓/减持/注资与 master ``SimplePortfolio`` 同序）；模式 A：不划拨真实资金；模式 C：无划拨、无换算，子系统直接下单
    * - 未映射参数
      - ``sys_use_self_tm`` 生效
      - 模式 C 一律共享父真实账户，该参数不再作为分层能力，忽略并 ``HKU_WARN``（与 master 效果一致）

迁移建议：

- 位置传参调用 ``PF_Simple(...)`` / ``PF_WithoutAF(...)`` **无需修改**（返回类型别名可承接）；
  ``PF_WithoutAF`` 已恢复为模式 C，存量"共享账户 + 子系统自身 MM 定量"的策略可直接复现重构前结果
  （已用 8 ETF 趋势布林带组合逐笔验证：129 买 / 62 卖与旧版本完全一致）。
- 在重构窗口内把 ``PF_WithoutAF`` 当作"模式 A（影子信号 + 父换算）"使用的代码，请改用
  ``PF_SignalAggregate(...)``（或别名 ``PF_ModeA``）。
- ``pf.run(query)`` **无需修改**：:meth:`MultiSystem.run` 提供 ``query`` 兼容重载，
  以市场交易日历（``StockManager.get_trading_calendar``，默认 SH）为驱动轴，语义等价 master ``Portfolio.run(query)``。
- 需要自定义驱动轴时，改用显式写法：
  ``ms.set_axis_mode("calendar")`` + ``ms.set_date_axis(...)`` + ``ms.run(kdata)``。
- 依赖 ``lastSuggestion()`` 的代码改用 ``System.to_suggestions()``（字段结构不同）。
- 新项目直接使用 :class:`MultiSystem` + :class:`AllocateFundsBase`。
