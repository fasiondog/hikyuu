.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

保证金模型
==========

保证金模型回答"开一笔仓要占用多少资金"，与交易成本（费用与利息）、账户记账（现金与持仓）分属三个组件：

- **保证金模型（本文）**：开仓占用多少资金、维持保证金阈值、空头是否需要借入标的；
- **交易成本**（见 :doc:`trade_cost`）：佣金 / 印花税 / 过户费 / 融资利息 / 借券费；
- **TradeManager**：现金、持仓、成交与权益记账，以及下单校验。

内建保证金模型
--------------

.. py:function:: MG_None()

    纯现金模型：不使用杠杆，开仓占用全额名义价值，持仓不计入保证金占用

    :return: :py:class:`MarginBase` 子类实例

.. py:function:: MG_Fixed([rate=0.6, rate_short=0.0, maintain_rate=0.0, margin_per_lot=0.0])

    固定保证金模型，对账户内所有标的生效。比例模式：``保证金 = rate × 价格 × 数量 × unit``；
    每手模式（``margin_per_lot > 0``）：``保证金 = margin_per_lot × ceil(数量 / 最小交易单位)``

    :param float rate: 开仓保证金比例，取值 (0, 1]
    :param float rate_short: 空头开仓比例，0 表示沿用 rate
    :param float maintain_rate: 维持保证金比例，0 表示与开仓保证金相同；非 0 时须 ≤ 开仓比例（高于开仓比例会在开仓瞬间即触发追保）
    :param float margin_per_lot: 每手保证金金额，0 表示使用比例模式
    :return: :py:class:`MarginBase` 子类实例

.. py:function:: MG_ByType([rates="", maintain_rate=0.0, default_rate=0.6])

    按证券类型 / 市场取保证金比例的模型，用于混合账户（如 A 股现金 + 期货保证金）

    :param str rates: ``键:比例`` 列表，以 ``,`` ``;`` 或空白分隔，如 ``"12:0.10, SHF:0.12"``；
        全数字键为证券类型数值，其余键为市场代码（大写比较）；类型命中优先于市场命中
    :param float maintain_rate: 维持保证金比例，0 表示与开仓保证金相同；非 0 时须 ≤ 开仓比例（高于开仓比例会在开仓瞬间即触发追保）
    :param float default_rate: 未命中任何键的标的所用比例，取值 (0, 1]；为 1.0 时未命中标的不计入保证金占用
    :return: :py:class:`MarginBase` 子类实例

.. note::

    多空共用一个比例。需要多空差异时使用 :py:func:`MG_Fixed` 的 ``rate_short``，或在 Python 中继承
    :py:class:`MarginBase` 覆写 :py:meth:`MarginBase.get_open_margin`。

自定义保证金模型
----------------

.. py:class:: MarginBase(name)

    保证金模型基类

    .. py:attribute:: name

        (只读) 模型名称

    .. py:method:: get_param(self, name)

        获取指定参数

        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: no such parameter

    .. py:method:: set_param(self, name, value)

        设置参数（参数在配置期设置，运行期按只读使用）

        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: clone(self)

        克隆操作

    .. py:method:: is_margin_trading(self, stock)

        【重载接口】该标的是否纳入保证金口径；false 时其持仓不计入占用

        :param Stock stock: 待判断证券
        :return: 是否计入保证金占用
        :rtype: bool

    .. py:method:: need_borrow_asset(self, stock)

        【重载接口】做空该标的是否需要借入标的。默认对内置证券类别返回 True（保留 A 股融券记账），
        数字币与自定义类型（>= 12，如期货、外汇、美股、港股）返回 False

        :param Stock stock: 待判断证券
        :return: 是否需要借券
        :rtype: bool

    .. py:method:: get_open_margin(self, date, stock, price, num, side=MarginSide.LONG)

        【重载接口·必需】开仓占用的保证金金额（本位币）

        :param Datetime date: 交易时刻
        :param Stock stock: 开仓标的
        :param float price: 开仓价格
        :param float num: 开仓数量，可传 1.0 以取得单位数量的占用
        :param MarginSide side: 多头或空头
        :return: 占用金额，输入非法时为 0
        :rtype: float

    .. py:method:: get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG)

        【重载接口】维持保证金（追保 / 强平阈值），默认等于开仓保证金

        :return: 维持保证金金额
        :rtype: float

    .. py:method:: get_margin_rate(self, date, stock, side=MarginSide.LONG)

        【重载接口】保证金的比例视图，取值 (0, 1]；每手模式下恒为 1.0

        :return: 保证金比例
        :rtype: float

    .. py:method:: get_max_open_number(self, date, stock, price, available, side=MarginSide.LONG)

        【重载接口】在给定可用资金下最多可开的数量，按最小交易单位向下取整并受最大下单量限制

        :param float available: 可用资金
        :return: 可开数量，无法开仓时为 0
        :rtype: float

    .. py:method:: _clone(self)

        【重载接口】子类克隆接口

.. py:class:: MarginSide

    开仓方向

    .. py:attribute:: LONG

        多头

    .. py:attribute:: SHORT

        空头

账户侧接口
----------

.. py:class:: TradeManager

    .. py:attribute:: margin

        账户的保证金模型（:py:class:`MarginBase`）。赋值为 None 时回落到纯现金模型 MG_None；
        新建账户默认为 MG_Fixed(0.6)，即与历史一致的 60% 购买力口径

    .. py:method:: get_open_margin(self, date, stock, price, num, side=MarginSide.LONG)

        按账户保证金模型计算开仓占用

    .. py:method:: get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG)

        按账户保证金模型计算维持保证金

    .. py:method:: get_max_open_number(self, date, stock, price, available, side=MarginSide.LONG)

        按账户保证金模型计算可用资金下的可开数量

    .. py:method:: get_occupied_margin(self, date, ktype="DAY")

        指定时刻多头与空头持仓占用的保证金合计（不计入 ``is_margin_trading`` 为 False 的持仓）

    .. py:method:: get_available_margin(self, date, ktype="DAY")

        可用于新开仓的资金，即 ``净资产 - 已占用保证金``

.. note::

    可用资金用净资产而非现金作口径：空头开仓会把卖出款计入现金，若以现金衡量则可无限开空。

    购买力判定只在 ``support_borrow_cash=True``（多头）或保证金模型纳入该标的（空头）时生效；纯现金账户
    （MG_None 且不开启融资）的买入 / 卖出判定与改造前完全一致。

保证金记账（Phase B）
---------------------

``margin_accounting`` 是账户级开关（默认 False）。开启后**保证金标的**（``is_margin_trading=True``
且 ``support_borrow_cash=True`` 或 ``need_borrow_asset=False``，即融资买入、期货、数字币）的开仓只
锁保证金，其余持仓（A 股现金买入等）仍按全额现金记账。

记账公式：

- 开多 / 开空：``现金 -= 保证金 + 费用``，``PositionRecord.margin += 保证金``；``buy_money`` /
  ``sell_money`` 仍累计名义金额，仅供报表
- 平多 / 平空：``现金 += 释放保证金 + 已实现盈亏 - 费用``；释放保证金按平仓数量比例，已实现盈亏按
  **金额口径**计算：``成交金额 - 剩余结算基准 × (平仓数量 / 当前持仓数量)``（空头取反），其中成交金额
  ``= 成交价 × 平仓数量 × unit``；结算基准即 ``PositionRecord.settle_basis``（逐日盯市时滚动到结算价，
  首次结算前等于开仓名义金额 ``open_basis``），与合约乘数无关地保持量纲一致；平仓按比例同时释放
  ``open_basis`` 与 ``settle_basis``、加仓只累加新的开仓名义金额，因此「部分平仓后再加仓」不会摊薄
  剩余持仓成本
- 权益：``净资产 = 现金 + 锁定保证金 + 浮动盈亏 - 借入资金``；保证金持仓的权益贡献记入
  ``FundsRecord.market_value``（多空均记入此项，空头浮动盈亏已含方向），``short_market_value`` 只保留
  现金口径空头名义市值
- 报表：``FundsRecord.occupied_margin`` 为锁定保证金合计；结算价用当日收盘价近似

.. note::

    ``get_occupied_margin()`` 是按给定价格重算的**保证金要求**（覆盖模型纳入的全部持仓，含按现金口径记账的
    部分——与券商把全部担保品计入一致），``FundsRecord.occupied_margin`` 是开仓时**已锁定**的金额；两者差异
    既含盯市漂移也含口径差异：``get_available_margin() = 净资产 - 保证金要求``。

    开仓的「保证金 + 手续费」与多头同口径：支持融资（``support_borrow_cash=True``）时差额自动借入并记入
    ``borrow_cash``，否则现金不足直接拒单，因此**开仓不会使现金为负**；只有「平仓亏损大于释放保证金」时才
    允许现金为负（真实透支），后续开仓由额度门拦截。

    ``auto_checkin``（``MoneyManager`` 参数，``System`` 据此在临下单前补款，``MultiSystem`` 的批次补款同口径）
    只补「本单实际需要的现金」：保证金记账的多头开仓补「保证金 + 费用」（其余走融资，不虚增本金），保证金记账
    的空头平仓完全不补（平空释放保证金并结算盈亏，透支合法）；现金口径的开仓与平仓仍补全额成交金额加费用。

    保证金要求不足一个最小货币单位（``10^-precision``，默认 0.01）时，锁定额取下限一个单位而不是取整为
    0：``margin > 0`` 是「该持仓按保证金口径记账」的标记，取整为零会让持仓退回现金口径估值（按全额名义市值
    计权益却几乎未付现金），与其实际承担的风险不符；``get_occupied_margin()`` 的合计要求不设此下限。

    ``add_trade_record`` 回填的交易记录只含成交、不含随附的融资凭证（``BUSINESS_BORROW_CASH``）：现金
    不足且 ``support_borrow_cash=True`` 时，按与实时 ``buy`` / ``sell_short`` 相同的迭代重新借入（融资费
    自身也计入），因此导出再导入的保证金开仓不会被误拒，重建的现金、负债与锁定保证金与实时账簿一致；
    ``support_borrow_cash=False`` 时仍按现金口径拒单。借券费同理在借券入账时一并融资。

    既缴保证金又需借券的开空先做只读校验、再统一融资、最后一次性借券入账：券源不足（未平空数量不够、
    无券可借、全部已卖出）或占位价开空都在**任何记账发生之前**返回，不会残留 ``borrow_cash`` 负债或融资费；
    借券费与保证金一并计入所需现金，不会出现「借多了、费用没着落」而导致现金为负。

    同一标的中途切换口径（先按现金口径建仓后再按保证金加仓，或 ``realPrice == 0`` 的占位开仓后补价成交）
    会形成**混合批次**：持仓级记账只有单一锁定保证金额与按金额比例摊薄的成本基数，估值与平仓资金流会把现金
    批次的成本并入保证金公式，与实盘分笔逐批的口径存在偏差；**任一方向**（保证金批次并入现金持仓、现金批次
    并入保证金持仓）发生时账户都会打告警。批次级（分笔）保证金与成本记账属后续立项。

    开仓锁定的保证金写进 ``TradeRecord.margin``（现金记账及非开仓业务恒为 0）：``add_trade_record``
    与历史回放优先用记录里的快照，只有快照为 0 的旧记录才按**当前**参数与保证金模型重算。因此建仓后再改
    ``margin_accounting``、更换模型或调整费率，回放结果仍与当时记账的结果一致；``PositionRecord.margin > 0``
    标记实时账簿中已按保证金口径记账的持仓。

    逐日结算、追保与强平见下文「逐日结算与风控闭环（Phase B-2）」；保证金持仓**不接受** ``checkin_stock`` /
    ``checkout_stock``（存/取会把成本基数改掉而锁定保证金不动，净资产会与资产脱节，故直接拒单，历史回填同样
    拒绝）；``realPrice == 0`` 的市价单占位在**开仓**时仍按现金口径记账；但保证金持仓不接受占位价平仓（返回无效
    交易，需传入成交价，否则锁定保证金无法释放）；历史回放遇到此类记录同样不释放锁定保证金并打告警。

    保证金模型对某标的返回 0 要求（或返回非有限值）时，该笔开仓退回现金口径并按全额名义金额记账（并发告警）：
    ``margin == 0`` 的持仓会被当作现金持仓估值，若不退单会变成「不付现金却计入全额市值」。

逐日结算与风控闭环（Phase B-2）
---------------------------------

保证金账户在每个 K 线周期的收盘阶段做一次**结算**（日线账户即逐日结算）：先把浮动盈亏过账成现金、把锁定额
重算到结算价，再用维持保证金检查权益；资金不足时告警，开启 ``force_liquidate`` 则同时挂出强平，由交易系统
在下一个开盘执行。相关参数（账户级 ``set_param``）：

- ``margin_accounting``（默认 False）：保证金记账开关，关闭时账户不做任何结算
- ``settle_with_close``（默认 True）：以该周期收盘价作为结算价；关闭后 ``settlement()`` 直接返回 False
  （暂未提供其他结算价来源）
- ``force_liquidate``（默认 False）：权益低于维持保证金要求时只告警；开启后把强制平仓挂入待执行队列

.. py:method:: TradeManager.settlement(date, ktype="DAY")

    对给定时刻结算保证金持仓，返回本次是否**完成过账**：已结算过的同一时刻再调用返回 True（结算幂等）；
    不做结算的账户（``margin_accounting`` / ``settle_with_close`` 关闭、未设保证金模型）返回 False；没有
    可结算持仓（空仓，或全部持仓被跳过）时本次未过账，同样返回 False，此时 ``is_settled`` 也为 False

    :param Datetime date: 结算时刻，不得早于 ``last_datetime``
    :param str ktype: 取结算价的 K 线类型

.. py:method:: TradeManager.is_settled(date)

    该时刻是否已结算。结算幂等：同一时刻重复调用不会重复过账

.. py:method:: TradeManager.is_forced_liquidation_queued(date)

    该时刻是否有待执行的强制平仓：结算每次都会重建队列（权益恢复即清除），挂出后为 True，直到持仓被
    清空；账户已无多空持仓时恒为 False。该探针回答的是「已挂出且仍有持仓待平」，探测本身不消费挂出
    日期，因此持仓清空后即使挂出日期还留着也不再返回 True

结算内容（逐个保证金持仓进行，现金口径持仓不参与）：

- 结算价：该时刻对应 K 线的收盘价；无 K 线（停牌等）时沿用最近有效收盘价，并在结算记录的 ``remark`` 标记
- 盯市：``现金 += ±(结算价 × 数量 × unit - settle_basis)``（多头取正、空头取反），随后 ``settle_basis``
  重置为结算价下的名义金额；因此结算后浮动盈亏从零重新累计，平仓的已实现盈亏按最新的盯市基准计算
- 重算保证金：按结算价重算锁定额，差额直接过账（``现金 -= 差额``）；差额为正且现金不足时按开仓同口径
  融资（``support_borrow_cash=True``），否则允许透支并告警——已经追保的账户不应因补不齐保证金而结算失败
- 维持保证金：``Σ get_maintain_margin(...)`` 覆盖**全部**保证金持仓——当日因停牌、越过最后注册 K 线等
  原因未过账的持仓同样计入（无有效价格时按 ``settle_basis`` 反推单价），否则这些持仓只计入权益而漏计要求；
  权益按 ``现金 + Σ持仓权益 - 借入资金`` 在该时刻估值，与 ``get_funds`` 同口径，低于要求即触发追保
  （首次触发时告警一次，恢复时提示已解除）
- 完成结算且 ``force_liquidate=True`` 时挂出强平，``is_forced_liquidation_queued()`` 转为 True

结算记录：每个被结算的持仓产生一条 ``BUSINESS_SETTLEMENT`` 记录（不是成交）——``real_price`` 为结算价，
``number`` 带方向（多头为正、空头为负，同一标的的多空持仓因此可分），``margin`` 为重算后的锁定保证金，
``cash`` 为过账后的现金余额，``cost`` 为 0，``part`` 为无效部件。

.. note::

    保证金记账关闭或 ``settle_with_close=False`` 时账户不做任何结算，A 股现金账户的账簿与改造前完全一致。

    ``System`` 在每个 K 线的收盘阶段按「交易 → 到期强平 → 逐日结算」的顺序推进：先由各部件完成本根 K 线的
    交易，再对保证金持仓做到期强平（持仓已越过标的最后注册 K 线的情形，G12，按收盘价成交），最后调用
    ``settlement()``；到期强平在结算之前，因此实现盈亏的成交先入账，结算再基于已平仓的账簿进行。若上一根 K
    线的结算挂出了强制平仓，``System`` 在本根 K 线开盘后、处理常规委托之前以开盘价平掉该标的的全部多空持仓，
    强平成交同样计入交易记录与持仓历史。

    ``MultiSystem`` 组合回测在组合层收盘阶段同样对父账户调用 ``settlement()``（模式 A/B 的父账户才是真实
    账簿，子账户只是影子）；模式 C 的父子共享同一账户，成员收盘时已结算，重复调用因幂等而不产生副作用。
    父层没有单一标的，因此挂出的强平在组合层开盘阶段按持仓逐个以开盘价平掉多空两侧，并清空子系统的买方
    延迟委托意图（与 ``System`` 丢弃延迟买入请求同口径，只删减仓方向）。

    历史回放不需要额外状态：结算记录本身就是账簿的一部分，``get_funds(date)``、``get_funds_list``、
    ``Performance`` 以及 ``add_trade_record`` 回填都会按记录还原当日结算后的现金、锁定额与盯市基准。

    ``save_action`` 生成的动作脚本**不包含**结算动作：脚本只回放成交，重新执行得到的是未结算的账簿，
    需要按周期自行重跑 ``settlement()``（结算记录可以参与 ``add_trade_record`` 回填，只是不写入动作脚本）。
