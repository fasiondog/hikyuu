.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _allocate_funds:

资产分配算法组件|AF
====================

资产分配算法组件（AllocateFunds，AF）用于对聚合系统 :class:`MultiSystem` 中选中的子系统进行
**组合级**资金分配。AF 的操作域是「子系统集合」（``SYSPtr`` 的集合），每个 :class:`MultiSystem`
持有一条 AF 实例（见 :doc:`portfolio`）。

.. note::

   **资金管理（MM）与资产分配（AF）职责分离**：

   * :class:`MoneyManagerBase` 限定为**单系统 / 单证券**形态（操作域为单个 ``Stock``），
     负责单系统 sizing；
   * 组合级分配（L1 系统级权重 / L2 行为级换算 / L3 组合风控）由
     :class:`AllocateFundsBase` 承载。

   组合级的资金调拨逻辑统一由 ``AF_*`` 承接。

分配模式（A / B / C）
---------------------

分配模式决定聚合系统的整体运行形态。模式由 :class:`MultiSystem` 自身持有（经 ``set_mode`` 设定、
经只读属性 ``mode`` 读取），并以末尾的 ``mode`` 形参显式传入 AF 的每个 L1/L2/L3 钩子；
AF 自身不持有该模式（见 :doc:`../trade_sys/system`）：

.. list-table::
    :header-rows: 1

    * - 模式
      - 名称
      - 子账户形态
      - 调仓与执行语义
    * - ``"A"``
      - 信号汇总（Signal Aggregation，**默认**）
      - 各子系统独立影子账户，恒运行、互不重叠
      - 父账户是**唯一决策者**：调仓日读取被选中子系统的**当前意图态**（净敞口 + 未兑现信号），
        按 L1 权重合成每标的**带符号净目标**（多空净额相互抵消），对父账户按差额统一下单
    * - ``"B"``
      - 资金配置（Fund Allocation）
      - 各子系统独立影子账户，从 0 起步、仅靠配额注资
      - 父账户是**资金分配者**：调仓日由 L1 计算各子系统**配额**（目标总资产）并校准
        （零配额清算 → 未选中清算/选中回收 → 减超额 → 注缺口）；子系统在各自配额内自主交易，
        父**连续镜像**其真实指令（主账户 = 各子系统头寸之和）
    * - ``"C"``
      - 共享账户兼容（Shared Account Compatibility）
      - 无影子账户，子系统直接在父真实账户交易
      - 兼容旧 ``Portfolio`` 语义（驱动集由 SE 语义决定）；父不做 L2 换算，AF 仅提供 L1/L2/L3 空操作（模式由聚合系统自身持有）

模式 × 算法部件对照：

.. list-table::
    :header-rows: 1

    * - 部件
      - 模式 A
      - 模式 B
      - 模式 C
    * - L1 系统级分配
      - 产名义权重 ``{System: weight}``（信号权重）
      - 产真实额度：就地回写 ``contexts[i].quota``（目标总资产），可穿透嵌套
      - 直通，不分配
    * - L2 行为级换算
      - 按 ``w × ratio × 父净值`` 合成每标的净目标并换算下单数量（带符号，先吐现金组后吃现金组）
      - 透传子系统指令（多空均透传）
      - 直通，免换算
    * - L3 组合风控
      - 组合风控裁剪（``max_single_position`` 集中度上限等）
      - 关闭或仅总量（``max_total_position`` 配额总和上限在 L1 承载）
      - 关闭且不可启用

.. note::

   模式 C 不是模式 B 的参数特例：无法通过模式 B 的任意参数档（``adjust_running_sys`` 取任意值、
   AF 不作分配、关闭父等比缩减等）等效得到。二者是**两种账户拓扑**——B 的子系统运行在**隔离的
   影子账户**内、由父连续镜像其成交；C 的子系统**直接共享父真实账户**（有意为之的例外）。
   共享账户下多个子系统**竞争同一现金池**，这也是模式 B 不具备的行为。

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
      - ``_allocate(date, tm, contexts, query, mode)``
      - 子系统上下文 → 权重 ``{System: weight}``；模式 B 下就地回写 ``contexts[i].quota``（真实额度）
    * - L2 行为级换算
      - ``_to_targets(date, tm, suggestions, sys_weight, query, mode)``
      - 权重 → 父账户可执行数量（模式 A 按「权重 × 带符号净目标比重 × 父净值」合成每标的净目标并 trade-the-delta；模式 B 透传子系统指令）
    * - L3 组合风控
      - ``_check_risk(date, tm, suggestions, query, mode)``
      - 组合级风控裁剪（如 ``max_single_position`` 集中度上限）

``_to_targets`` 与 ``_check_risk`` 均有默认实现，通常只需重载 ``_allocate``。

公共参数（组合级）
------------------

基类 :class:`AllocateFundsBase` 内置以下公共参数：

.. list-table::
    :header-rows: 1

    * - 参数
      - 类型与默认值
      - 生效模式
      - 说明
    * - ``max_single_position``
      - float，1.0
      - 仅 A
      - L3 单标的集中度上限：单标的目标持仓市值 ≤ 净资产 × 该值。≤0 表示不限制；大于 1 时按 1.0 生效
    * - ``max_total_position``
      - float，1.0
      - A / B
      - 总预算上限：模式 A 下限定 L2 全部标的聚合目标市值 ≤ 净资产 × 该值；
        模式 B 下限定 L1 各子系统**配额总和** ≤ 净资产 × 该值。
        两种口径均**包含** ``fixed_amount`` 定价部分：权重定价按**等比缩减**，固定额度按**整数个额度**
        截断（详见下）。≤0 表示不限制；>1 即允许杠杆
    * - ``fixed_amount``
      - float，0.0
      - A / B
      - 定价开关：大于 0 时**优先于权重定价**并告警。模式 A 下为每标的目标市值；
        模式 B 下为每子系统固定额度。此时 L1 权重仅保留排序含义、不参与定价。
        合计额度仍受 ``max_total_position`` 约束
    * - ``adjust_running_sys``
      - bool，True
      - 仅 B
      - 调仓日是否将**已运行子系统**强制调整到新配额。已运行子系统 = 上期已获注资、
        校准时刻仍持有资产的子系统。``True``：执行完整校准（零配额清算 → 未选中清算/选中回收 → 减超额 → 注缺口）；
        ``False``：已运行子系统保持现状（不回收、不减持、不注资），
        仅新子系统获配剩余父账户现金（受父账户余额限制，可能不足额）。
        模式 A 每 bar 重定目标、恒调整；模式 C 无配额、恒不调整——两者均不受该参数影响

注意事项：

.. list-table::
    :header-rows: 1

    * - 事项
      - 说明
    * - 评分归一化后**满仓分配**
      - ``AF_MultiFactor`` 默认（``auto_adjust_weight=True``）将评分**归一化为和为 1** 后再分配，
        因此**评分总和小于 1 时也会满仓分配**（评分仅表达子系统间的相对比例）。
        设为 ``False`` 时评分即最终权重，超出总预算的部分依赖 ``max_total_position`` 等比缩减
    * - 恒定权重不触发减仓
      - 恒定权重类 AF（如 ``AF_FixedWeightList``）每期按**当期父净资产**重算配额，
        子账户净值与配额同比例增长，正常行情下不会触发减仓；
        强制减仓只在权重或资金**跨期收缩**时发生（或 ``adjust_running_sys=False`` 时完全不发生）
    * - ``fixed_amount`` 受总预算上限约束（按额度个数截断）
      - 固定额度是**绝对金额**，超预算时不做等比缩减（否则额度会随净资产与子系统数漂移），
        而是**只满足整数个额度**：模式 B 按子系统顺序发放完整 ``fixed_amount``，额度耗尽后其余子系统
        配额置 0；模式 A 按建议中出现顺序发放，预算不足的标的目标置 0（即平掉该标的）。
        若单份额度已超过上限（额度 > 净资产 × ``max_total_position``），则不会有任何获配
        （告警显示 0/N），此时应调小 ``fixed_amount`` 或调大上限。
        模式 B 下配额为 0 的子系统按 ``sell_at_not_selected`` / ``adjust_running_sys`` 的处理规则执行
        （前者为真时清仓并停止驱动，后者为假时已运行子系统保持不动）。
        注：AF 层不再等比缩减固定额度，但执行端仍受父账户资金约束：融资账户（``support_borrow_cash``，
        需设在 ``MultiSystem`` 上、由 ``readyForRun`` 下发给父 TM）按**保证金率逐笔**折算购买力
        ——单笔可超出可用现金；同一批多笔则按顺序分配，靠后的腿可能被缩减（与 TradeManager 的逐笔
        融资模型一致），回补空头需全额现金、不适用保证金折算。非融资账户按可用现金等比压缩。
        ``MultiSystem`` 另有 ``auto_checkin``（默认 ``False``）：按“名义额 + 手续费”计算缺口并向父账户
        **追加本金**（与融资开关组合时也按全额补足，使所有腿无需借款即可成交），仅用于保证计划数量
        可成交；它追加本金而非负债，回测资金曲线会失真，且目标逐日变化时可能逐日补资。
        需要维持原额度或使用杠杆时，直接调大 ``max_total_position``（0 表示不限）
    * - 模式 A 下 ``fixed_amount`` 按标的只应用一次
      - 模式 A 的固定额**按标的**定价而非按建议条数：同一标的被 N 个子持有时只取聚合后的带符号方向，
        每个标的仅应用一次 ``±fixed_amount`` 目标（不会放大成 N 倍）。方向冲突时相消，
        净额为零则驱动该标的平仓
    * - 同一标的的计划价取首条
      - 模式 A 用建议的 ``plan_price`` 把聚合目标市值换算成股数：同一标的多条建议给出不同计划价时
        **以首条为准**，其余仅记 debug 日志。计划价应来自同一 bar、同一价格口径
        （同一复权方式与滑点），否则成交股数会与目标市值不符
    * - 模式 A 只重定"本轮建议覆盖"的标的
      - 模式 A 以本轮汇总的建议为目标快照：只对**建议中出现的标的**（子系统当日成交的标的，
        以及 MultiSystem 对未选中系统注入的清仓腿）换算并 trade-the-delta；
        **未出现的标的保持父账户现有持仓不动**。AF 假定父账户是各子系统账本的镜像，
        不负责发现和清理与任何子系统都不对应的持仓（如直接在父账户手工交易、
        或把未清算的子系统移出列表）
    * - 无权重建议被跳过
      - 模式 A 的权重表以 **System 实例**为键：建议的 ``sys`` 必须是 L1 上下文中同一个实例。
        权重表为空、表中缺该子系统、或上游返回了克隆实例（与上下文实例不同）时，该建议
        **告警并跳过**，不会按等价权重兜底定价；父账户自行合成的建议（``sys`` 为空，
        如未选中标的的零目标）不受影响。``fixed_amount>0`` 时不做权重查表（绝对额度即目标），
        本条不适用
    * - L3 只收缩、不重新分配
      - ``max_total_position``（总预算）与 ``max_single_position``（单标的集中度）依次生效，
        两处都只**收缩**超额部分：L3 裁剪释放出的预算**不会**再分配给其它标的，
        因此最终总敞口可能明显低于 ``max_total_position``（如 10 个等权标的、单标的上限 0.05 时，
        总敞口仅 0.5 × 净资产）


内建资产分配算法
------------------

.. py:function:: AF_EqualWeight()

    等权重资产分配，对选中的资产进行等比例分配（L1 等权 1/N）。

    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_FixedWeight([weight = 0.1])

    固定比例资产分配，每个选中的资产都只占总资产固定的比例。

    L1 直接返回固定 ``weight``，**不归一化**（评分即最终权重的用法见 ``AF_MultiFactor`` 的
    ``auto_adjust_weight=False``）。

    :param float weight: 指定的资产比例 (0, 1]
    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_FixedWeightList(weights)

    固定比例资产分配列表，按指定的权重列表对选中系统进行资产分配。

    L1 按子系统顺序逐一取 ``weights[i]``，**不归一化**；数量不符时回退等权。
    空列表无法定价，工厂与参数校验均直接抛异常（不再降级为等权）。
    权重须为有限值、非负且不全为 0（全 0 会使各子系统目标市值归零、驱动父账户每次调仓主动清仓现有持仓，
    故参数校验阶段抛出异常）。

    :param list weights: 指定的资产比例列表
    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_FixedAmount([amount = 20000.0])

    固定金额资产分配（L1 等权 + L2 固定金额），对选中的资产进行等金额分配。

    当前实现要求 ``fixed_amount >= 0``，且按净额调仓。

    :param float amount: 交易最大金额
    :return: 资产分配算法实例（``AFPtr``）

.. py:function:: AF_MultiFactor([auto_adjust_weight = True])

    创建 MultiFactor 评分权重的资产分配算法实例，即以 SE 返回的评分
    （``SubSystemContext.score``）作为权重（非有限评分按 0 处理，全 0 或求和非有限时回退等权）。

    :param bool auto_adjust_weight: 评分归一化开关（默认 ``True``）。``True`` 时将评分归一化为
        和为 1 再分配（评分总和小于 1 时同样满仓分配，见上文注意事项）；
        ``False`` 时评分即最终权重（不归一化），超出总预算的部分依赖 ``max_total_position`` 缩减
    :return: 资产分配算法实例（``AFPtr``）


自定义资产分配算法
------------------

推荐用 ``crtAF`` 快速构造自定义 AF：只需给出 L1 分配回调，L2 / L3 按需注入。

.. py:function:: crtAF(allocate_func[, params, name, to_targets_func, check_risk_func])

    快速创建资产分配算法

    :param allocate_func: L1 系统级分配接口 ``func(self, date, tm, contexts, query, mode)``，
        返回 ``{System: weight}``；模式 B 下就地回写 ``contexts[i].quota`` 并返回空表。``mode`` 为聚合系统下传的运行模式（"A"/"B"/"C"），模式由聚合系统持有
    :param {} params: 参数字典
    :param str name: 自定义名称
    :param to_targets_func: L2 行为级换算接口 ``func(self, date, tm, suggestions, sys_weight, query, mode)``，可选
    :param check_risk_func: L3 组合风控接口 ``func(self, date, tm, suggestions, query, mode)``，可选
    :return: 自定义资产分配算法实例（``AFPtr``）

下例的 ``my_allocate`` 同时演示模式 A（返回权重表）与模式 B（回写额度）：

.. code-block:: python

    from hikyuu import *

    def my_allocate(self, date, tm, contexts, query, mode):
        # L1【必须】：子系统上下文 → 权重
        #   模式 A：返回 {ctx.sys: weight}
        #   模式 B：直接写 ctx.quota，并返回空表
        #   mode 为聚合系统下传的运行模式（"A"/"B"/"C"）
        if mode == "B":
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


资产分配算法基类
----------------

.. py:class:: AllocateFundsBase

    资产分配算法（AF）基类，仅聚合系统（MultiSystem）使用。

    .. py:attribute:: name  名称
    .. py:attribute:: tm    设置或获取交易管理对象
    .. py:attribute:: query 设置或获取查询条件

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

    .. py:method:: allocate(self, date, tm, suggestions, contexts, query, mode)

        L1/L2/L3 统一入口（通常由 :class:`MultiSystem` 内部调用）：先执行 ``_allocate`` 得到权重，
        再经 ``_to_targets`` 换算为可执行数量，最后经 ``_check_risk`` 组合风控裁剪。

        .. note::

            **仅供 MultiSystem 内部调用，不属于面向用户的 API**：它假定 ``contexts`` /
            ``suggestions`` / ``mode`` 已由聚合系统在调仓日准备就绪。手工构造参数直接调用只会得到
            无意义的换算结果——既不下单，也不影响任何回测；请改为驱动 :class:`MultiSystem`。
            自定义算法应重载 ``_allocate`` / ``_to_targets`` / ``_check_risk``，而非调用本入口。

        :param Datetime date: 分配日期
        :param TradeManager tm: 父账户交易管理
        :param TradeSuggestionList suggestions: 子系统交易建议（就地改写）
        :param list contexts: 子系统上下文（``SubSystemContext``；模式 B 下就地回写 ``quota``）
        :param KQuery query: 查询条件
        :param str mode: 运行模式（"A"/"B"/"C"），由聚合系统持有

    .. py:method:: _allocate(self, date, tm, contexts, query, mode)

        【重载接口】L1 系统级分配：由子系统上下文决定各子系统权重。

        模式 A 返回 ``{System: weight}``；模式 B 就地回写 ``contexts[i].quota`` 并返回空表。

        :param str mode: 运行模式（"A"/"B"/"C"），由聚合系统持有
        :return: 权重表（``{System: weight}``）

    .. py:method:: _to_targets(self, date, tm, suggestions, sys_weight, query, mode)

        【重载接口】L2 行为级换算：将权重换算为父账户可执行数量，就地改写 ``suggestions``。

        模式 A 读取子系统当前意图态（``target_position_ratio`` 带符号净目标），按
        ``w × ratio × 父净值`` 合成每标的净目标并对父账户按差额（trade-the-delta）下单；
        模式 B 透传子系统指令并做防御性裁剪。未重载时使用基类默认实现。

        :param str mode: 运行模式（"A"/"B"/"C"），由聚合系统持有

    .. py:method:: _check_risk(self, date, tm, suggestions, query, mode)

        【重载接口】L3 组合风控裁剪，就地改写 ``suggestions`` 数量。

        基类默认实现按 ``max_single_position`` 限制单标的集中度（模式 B 下跳过）。
        未重载时使用基类默认实现。

        :param str mode: 运行模式（"A"/"B"/"C"），由聚合系统持有

    .. py:method:: _reset(self)

        【重载接口】子类复位接口，复位内部私有变量
