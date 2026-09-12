.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _allocate_funds:

资产分配算法组件|AF
====================

资产分配算法组件（AllocateFunds，AF）用于对聚合系统 :class:`MultiSystem` 中选中的子系统进行
**组合级**资金分配。AF 的操作域是「子系统集合」（``SYSPtr`` 的集合），每个 :class:`MultiSystem`
持有一条 AF 实例（见 :doc:`portfolio`）。

.. note::

   自 ``feature/next`` 起，**资金管理（MM）与资产分配（AF）职责分离**：

   * :class:`MoneyManagerBase` 限定为**单系统 / 单证券**形态（操作域为单个 ``Stock``），
     负责单系统 sizing，不再承载组合级分配；
   * 组合级分配（L1 系统级权重 / L2 行为级换算 / L3 组合风控）整体迁移至本研究主题
     :class:`AllocateFundsBase`。

   即 master 中 `MM_FixedWeight` / `MM_FixedWeightList` / `MM_MultiFactor` 等组合级 MM、
   以及独立的 ``Portfolio`` 资金调拨逻辑，现由 ``AF_*`` 承接。

AF 的三个可替换算法部件
-----------------------

AF 把组合级分配拆成三个**可独立替换**的算法部件（各自对应基类的一个重载接口），
并由 :meth:`AllocateFundsBase.allocate` 串成一条管线：

.. list-table::
    :header-rows: 1

    * - 部件
      - 重载接口
      - 语义
    * - L1 系统级分配
      - ``_allocate(date, tm, contexts, query)``
      - 子系统上下文 → 权重 ``{System: weight}``；模式 B 下就地回写 ``contexts[i].quota``（真实额度）
    * - L2 行为级换算
      - ``_to_targets(date, tm, suggestions, sys_weight, query)``
      - 权重 → 父账户可执行数量（模式 A 按比重换算下单数量；模式 B 透传子系统指令）
    * - L3 组合风控
      - ``_check_risk(date, tm, suggestions, query)``
      - 组合级风控裁剪（如 ``max-single-position`` 集中度上限）

``_to_targets`` 与 ``_check_risk`` 均有默认实现，通常只需重载 ``_allocate``。

公共参数（组合级）
------------------

    * **weight-list** *(string|"")* : L1 固定权重列表（逗号分隔）；非空且数量与子系统数一致时按序采用，
      否则回退等权，权重和不为 1 时自动归一化。
    * **fixed-amount** *(float|0.0)* : L2 固定金额；模式 A 下为每标的目标市值，模式 B 下为每子系统固定额度。
    * **max-single-position** *(float|1.0)* : L3 单标的集中度上限，目标持仓市值 ≤ 总资产 × 该值
      （≤0 或 ≥1 表示不限制）。

.. note::

   master 的 ``adjust_running_sys`` / ``auto_adjust_weight`` / ``ignore_zero_weight`` / ``reserve_percent``
   等参数在 v6 **无独立开关**：其中 ``auto_adjust_weight`` 的语义由 ``AF_FixedWeight`` /
   ``AF_FixedWeightList`` 的「**不归一化**」直接承载；其余参数无对应，需由使用方自行处理。


内建资产分配算法
------------------

.. py:function:: AF_EqualWeight()

    等权重资产分配，对选中的资产进行等比例分配（L1 等权 1/N）。

    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_FixedWeight([weight = 0.1])

    固定比例资产分配，每个选中的资产都只占总资产固定的比例。

    L1 直接返回固定 ``weight``，**不归一化**（等价 master ``auto_adjust_weight=False``）。

    :param float weight: 指定的资产比例 (0, 1]
    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_FixedWeightList(weights)

    固定比例资产分配列表，按指定的权重列表对选中系统进行资产分配。

    L1 按子系统顺序逐一取 ``weights[i]``，**不归一化**；数量不符时回退等权。

    :param list weights: 指定的资产比例列表
    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_FixedAmount([amount = 20000.0])

    固定金额资产分配（L1 等权 + L2 固定金额），对选中的资产进行等金额分配。

    .. note::

       master 旧实现断言 ``amount > 500`` 且剩余现金 ``< 0.6 × amount`` 时停止；
       v6 实现要求 ``fixed-amount >= 0``，且按净额调仓，**边界行为不同**。

    :param float amount: 交易最大金额
    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_MultiFactor()

    创建 MultiFactor 评分权重的资产分配算法实例，即直接以 SE 返回的评分
    （``SubSystemContext.score``）作为权重。

    :return: 资产分配算法实例（``AFPtr``）


自定义资产分配算法
------------------

推荐用 ``crtAF`` 快速构造自定义 AF：只需给出 L1 分配回调，L2 / L3 按需注入。

.. py:function:: crtAF(allocate_func[, params, name, to_targets_func, check_risk_func])

    快速创建资产分配算法

    :param allocate_func: L1 系统级分配接口 ``func(self, date, tm, contexts, query)``，
        返回 ``{System: weight}``；模式 B 下就地回写 ``contexts[i].quota`` 并返回空表
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param to_targets_func: L2 行为级换算接口 ``func(self, date, tm, suggestions, sys_weight, query)``，可选
    :param check_risk_func: L3 组合风控接口 ``func(self, date, tm, suggestions, query)``，可选
    :return: 自定义资产分配算法实例（``AFPtr``）

下例的 ``my_allocate`` 同时演示模式 A（返回权重表）与模式 B（回写额度）：

.. code-block:: python

    from hikyuu import *

    def my_allocate(self, date, tm, contexts, query):
        # L1【必须】：子系统上下文 → 权重（对应 master 的 _allocateWeight）
        #   模式 A：返回 {ctx.sys: weight}
        #   模式 B：直接写 ctx.quota，并返回空表
        if self.mode == "B":
            quota = tm.get_funds(date, query).total_assets / max(len(contexts), 1)
            for ctx in contexts:
                ctx.quota = quota
            return {}
        return {ctx.sys: 1.0 / len(contexts) for ctx in contexts}

    af = crtAF(my_allocate)

    # L2【可选】与 L3【可选】按需注入（未注入时使用基类默认实现）：
    # af = crtAF(my_allocate, to_targets_func=my_to_targets, check_risk_func=my_check_risk)

等效写法是直接继承 :class:`AllocateFundsBase`，实现 ``_clone`` 并按需重载
``_allocate`` / ``_to_targets`` / ``_check_risk``。

迁移对照：

.. list-table::
    :header-rows: 1

    * - master
      - v6
    * - ``class MyAF(AllocateFundsBase)``
      - ``class MyAF(AllocateFundsBase)``（同名类保留，语义不变）
    * - ``_allocateWeight(date, se_list)``
      - ``_allocate(date, tm, contexts, query)``（L1，系统级权重 / 额度）
    * - （由框架后处理归一化）
      - ``_to_targets(...)``（L2，模式 A 换算父账户数量；模式 B 透传）
    * - —
      - ``_check_risk(...)``（L3，组合风控裁剪，如 ``max-single-position``）
    * - ``crtAF(func)``
      - ``crtAF(func)``（仅回调签名更新为 ``_allocate(date, tm, contexts, query)``，可另加 ``to_targets_func`` / ``check_risk_func``）


资产分配算法基类
----------------

.. py:class:: AllocateFundsBase

    资产分配算法（AF）基类，仅聚合系统（MultiSystem）使用。

    .. py:attribute:: name  名称
    .. py:attribute:: tm    设置或获取交易管理对象
    .. py:attribute:: query 设置或获取查询条件
    .. py:attribute:: mode  分配模式：``"A"``（信号汇总 + 父统一下单）或 ``"B"``（额度划拨 + 下期额度回写）

    .. py:method:: __init__(self[, name="AllocateFundsBase"])

        初始化构造函数

        :param str name: 名称

    .. py:method:: get_param(self, name)

        获取指定的参数

        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数

    .. py:method:: set_param(self, name, value)

        设置参数

        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: have_param(self, name)

        是否存在指定参数

        :param str name: 参数名称
        :return: 是否存在
        :rtype: bool

    .. py:method:: reset(self)

        复位操作

    .. py:method:: clone(self)

        克隆操作

    .. py:method:: allocate(self, date, tm, suggestions, contexts, query)

        L1/L2/L3 统一入口（通常由 :class:`MultiSystem` 内部调用）：先执行 ``_allocate`` 得到权重，
        再经 ``_to_targets`` 换算为可执行数量，最后经 ``_check_risk`` 组合风控裁剪。

        :param Datetime date: 分配日期
        :param TradeManager tm: 父账户交易管理
        :param TradeSuggestionList suggestions: 子系统交易建议（就地改写）
        :param list contexts: 子系统上下文（``SubSystemContext``；模式 B 下就地回写 ``quota``）
        :param KQuery query: 查询条件

    .. py:method:: _allocate(self, date, tm, contexts, query)

        【重载接口】L1 系统级分配：由子系统上下文决定各子系统权重。

        模式 A 返回 ``{System: weight}``；模式 B 就地回写 ``contexts[i].quota`` 并返回空表。

        :return: 权重表（``{System: weight}``）

    .. py:method:: _to_targets(self, date, tm, suggestions, sys_weight, query)

        【重载接口】L2 行为级换算：将权重换算为父账户可执行数量，就地改写 ``suggestions``。

        模式 A 按 ``assets_ratio`` 换算下单数量（超配转减仓、SELL/CLEAR 全平）；
        模式 B 透传子系统指令并做防御性裁剪。未重载时使用基类默认实现。

    .. py:method:: _check_risk(self, date, tm, suggestions, query)

        【重载接口】L3 组合风控裁剪，就地改写 ``suggestions`` 数量。

        基类默认实现按 ``max-single-position`` 限制单标的集中度（模式 B 下跳过）。
        未重载时使用基类默认实现。

    .. py:method:: _reset(self)

        【重载接口】子类复位接口，复位内部私有变量
