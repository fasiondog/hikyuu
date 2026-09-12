.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

投资组合|PF
=============

在 Hikyuu 中，投资组合以系统交易策略为单位，可以使用不同目标、同一交易策略的组合。

.. note::

   自 ``feature/next`` 起，原独立的 ``Portfolio``（``SimplePortfolio`` / ``WithoutAFPortfolio``）已由
   :class:`MultiSystem` 承接：**PF 本质就是 MultiSystem 的一个预置配置**。
   本页描述的 ``PF_Simple`` / ``PF_WithoutAF`` 是**兼容层工厂**，用于承接 master 存量调用方式，
   其**返回值为** :class:`MultiSystem`，不再是 ``Portfolio`` 对象。
   新项目建议直接使用 :class:`MultiSystem`（见 :doc:`../trade_sys/system`）。

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

    创建一个多标的、单系统策略的投资组合（**返回 MultiSystem，语义为模式 B：额度划拨 + 下期额度回写**）。

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

    创建无资金分配算法的投资组合（**返回 MultiSystem，语义为模式 A：信号汇总 + 父统一下单**）。

    调仓模式 adjust_mode 说明同上。

    :param TradeManager tm: 交易管理
    :param SelectorBase se: 交易对象选择算法
    :param int adjust_cycle: 调仓周期
    :param str adjust_mode: 调仓模式 "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: 如果当日不是交易日将会被顺延至当前周期内的第一个交易日
    :param bool trade_on_close: 交易是否在收盘时进行
    :param bool sys_use_self_tm: 原型系统使用自身附带的 tm 进行计算（**v5 无对应语义，忽略并告警**）
    :param bool sell_at_not_selected: 调仓日未选中的标的是否强制卖出
    :rtype: MultiSystem


与 master 的差异与迁移
----------------------

.. list-table::
    :header-rows: 1

    * - 维度
      - master
      - v5（工厂直通）
    * - 返回类型
      - ``PortfolioPtr``
      - ``MultiSystemPtr``（兼容别名 ``PortfolioPtr`` 指向 ``MultiSystemPtr``，存量 ``PortfolioPtr pf = PF_Simple(...)`` 可继续编译）
    * - 类 / 方法
      - ``Portfolio`` / ``SimplePortfolio`` / ``WithoutAFPortfolio`` 及其方法（``run`` / ``getRunningDates`` / ``getCycleEndDates`` / ``lastSuggestion`` …）
      - 不再存在；改用 :class:`MultiSystem` 的 ``run`` / ``getAdjustDates`` / ``getAdjustTurnover`` / ``toSuggestions``。
        其中 ``run(query)`` 另提供兼容重载（等价 master ``Portfolio.run(query)``，以市场交易日历为驱动轴），存量 ``pf.run(query)`` 无需改写
    * - 账户层级
      - 真实 TM + 影子 TM + 子系统账户
      - 父真实 TM + 子系统影子账户 ``TM_SUB``（:meth:`MultiSystem.set_sub_init_cash`）
    * - 资金调拨
      - 调仓日 checkout / checkin
      - 模式 B：下期额度回写（滞后一期）；模式 A：不划拨
    * - 未映射参数
      - ``sys_use_self_tm`` 生效
      - 忽略并 ``HKU_WARN``

迁移建议：

- 位置传参调用 ``PF_Simple(...)`` / ``PF_WithoutAF(...)`` **无需修改**（返回类型别名可承接）。
- ``pf.run(query)`` **无需修改**：:meth:`MultiSystem.run` 提供 ``query`` 兼容重载，
  以市场交易日历（``StockManager.get_trading_calendar``，默认 SH）为驱动轴，语义等价 master ``Portfolio.run(query)``。
- 需要自定义驱动轴时，改用显式写法：
  ``ms.set_axis_mode("calendar")`` + ``ms.set_date_axis(...)`` + ``ms.run(kdata)``。
- 依赖 ``lastSuggestion()`` 的代码改用 ``System.to_suggestions()``（字段结构不同）。
- 新项目直接使用 :class:`MultiSystem` + :class:`AllocateFundsBase`。
