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
