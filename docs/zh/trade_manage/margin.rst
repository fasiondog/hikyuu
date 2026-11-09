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
--------------------

``margin_accounting`` 是账户级开关（默认 False）。开启后**保证金标的**（``is_margin_trading=True``
且 ``support_borrow_cash=True`` 或 ``need_borrow_asset=False``，即融资买入、期货、数字币）的开仓只
锁保证金，其余持仓（A 股现金买入等）仍按全额现金记账。

记账公式：

- 开多 / 开空：``现金 -= 保证金 + 费用``，``PositionRecord.margin += 保证金``；``buy_money`` /
  ``sell_money`` 仍累计名义金额，仅供报表
- 平多 / 平空：``现金 += 释放保证金 + 已实现盈亏 - 费用``；释放保证金按平仓数量比例，已实现盈亏按
  **金额口径**计算：``成交金额 - 剩余成本基数 × (平仓数量 / 当前持仓数量)``（空头取反），其中成交金额
  ``= 成交价 × 平仓数量 × unit``，剩余成本基数即 ``PositionRecord.open_basis``，与合约乘数无关地保持量纲一致；
  平仓只按比例释放成本基数、加仓只累加新的开仓名义金额，因此「部分平仓后再加仓」不会摊薄剩余持仓成本
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
    并入保证金持仓）发生时账户都会打告警。批次级（分笔）保证金与成本记账、把锁定
    保证金写进 ``TradeRecord`` 属后续立项。

    历史回放（``getFunds(date)``、``get_funds_list``、``Performance``）与 ``add_trade_record``
    按**当前**参数与保证金模型重算保证金：建仓后再改 ``margin_accounting``、更换模型或调整费率，回放结果会
    与当时记账的结果不同；``PositionRecord.margin > 0`` 只标记实时账簿中已按保证金口径记账的持仓。

    到期追保与强平（Phase B-2）、每日变动保证金过账尚未实现；保证金持仓**不接受** ``checkin_stock`` /
    ``checkout_stock``（存/取会把成本基数改掉而锁定保证金不动，净资产会与资产脱节，故直接拒单，历史回填同样
    拒绝）；``realPrice == 0`` 的市价单占位在**开仓**时仍按现金口径记账；但保证金持仓不接受占位价平仓（返回无效
    交易，需传入成交价，否则锁定保证金无法释放）；历史回放遇到此类记录同样不释放锁定保证金并打告警。

    保证金模型对某标的返回 0 要求（或返回非有限值）时，该笔开仓退回现金口径并按全额名义金额记账（并发告警）：
    ``margin == 0`` 的持仓会被当作现金持仓估值，若不退单会变成「不付现金却计入全额市值」。
