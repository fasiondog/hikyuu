.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

资金管理策略|MM
===============

公共参数：

    * **auto_checkin=False** *(bool)* : 当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中补充存入（checkin）足够的现金；注资由系统在实际成交时一次性完成，数量查询本身不改变账户。
    * **max_stock=20000** *(int)* : 最大持有的证券种类数量（即持有几只股票，而非各个股票的持仓数）
    * **disable_ev_force_clean_position=False** *(bool)* : 禁用市场环境失效时强制清仓
    * **disable_cn_force_clean_position=False** *(bool)* : 禁用系统有效条件失效时强制清仓

    .. note::

        所有参数命名统一为小写下划线风格（如 ``auto_checkin``、``max_stock``、``max_loss``）。
        早期版本的连字符参数名（``auto-checkin``、``max-stock``）已更名，使用旧名称将抛出异常。

    .. note::
       资金管理策略实例内部保存每个标的的连续买入/卖出计数（``current_buy_count`` / ``current_sell_count``，
       档位型策略如 :func:`MM_FixedCountTps` 依赖它推进档位），该计数无锁。``System`` 的 ``shared_mm=True``
       会让多个系统并发写同一实例，并行回测时产生数据竞争、档位错乱；请为每个系统保留独立实例
       （``shared_mm=False``，默认值）。


内建资金管理策略
----------------

不做资金管理策略
^^^^^^^^^^^^^^^^^^

.. py:function:: MM_Nothing()

    特殊的资金管理策略，相当于不做资金管理，有多少钱买多少；开空时按净权益融多少卖多少。

    参数（通过 ``set_param`` 设置）：

    * ``if_have_a_position_will_not_buy``：已有多头持仓时不再买入，默认 ``False``
    * ``if_have_short_will_not_sell_short``：已有空头持仓时不再开空，默认 ``False``


固定交易数量资金管理策略
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCount([n = 100])

    固定交易数量资金管理策略。每次买入固定的数量。

    :param float n: 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
    :return: 资金管理策略实例

.. py:function:: MM_FixedCountTps([buy_counts, sell_counts])

    连续买入/卖出固定数量资金管理策略。

    止损价被击穿（每股风险 <= 0）、卖出档位用尽或卖出数量超过当前持仓（如除权后）时，卖出全部剩余持仓。

    每档数量必须是有限的严格正数（不得为 NaN/Inf/0/负数），若少一档请使用更短的列表；buy_counts 不能为空（至少需要一档）。

    sell_counts 允许为空：空列表被视为"卖出档位立即用尽"，首笔卖出信号即全部平仓。

    :param list buy_counts: 买入数量列表
    :param list sell_counts: 卖出数量列表
    :return: 资金管理策略实例


固定风险资金管理策略
^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedRisk([risk = 1000.00])

    固定风险资金管理策略对每笔交易限定一个预先确定的或者固定的资金风险，如每笔交易固定风险1000元。公式：交易数量 = 固定风险 / 交易风险。

    :param float risk: 固定风险
    :return: 资金管理策略实例


固定资本资金管理策略
^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCapital([capital = 10000.0])

    固定资金管理策略, 即控制每次买入投入的总资金。买入数量 = 当前可用现金 / capital

    :param float capital: 固定资本单位
    :return: 资金管理策略实例

.. py:function:: MM_FixedCapitalFunds([capital = 10000.0])

    固定资本管理策略。买入数量 = 当前净权益 / capital。基数为账户净权益（现金 + 持仓市值 − 空头市值 − 借券现金）而非可用现金，适用于按权益等比加仓的场景

    :param float capital: 固定资本单位
    :return: 资金管理策略实例


固定单位资金风险管理策略
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedUnits([n = 33])

    固定单位资金管理策略。公式: 买入数量 = max(当前现金, 初始资金) / n / 当前风险risk，即现金低于初始资金时按初始资金计算

    :param int n: n个资金单位
    :return: 资金管理策略实例


威廉斯固定风险资金管理策略
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    威廉斯固定风险资金管理策略，买入数量 =（当前可用现金 × 风险百分比p）÷ 最大损失(max_loss)

    :param float p: 风险百分比
    :param float max_loss: 最大损失
    :return: 资金管理策略实例


固定百分比资金管理策略
^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedPercent([p = 0.03])

    固定百分比风险模型。公式：P（头寸规模）＝ 当前可用现金 * 百分比 / R（每股的交易风险）。[BOOK3]_, [BOOK4]_ .

    :param float p: 百分比
    :return: 资金管理策略实例


多空交易能力
^^^^^^^^^^^^^^^^^^^^

多头方向（买入建仓、卖出平仓）各策略均支持。空头方向默认"能平不能开"：

.. list-table::
   :header-rows: 1

   * - 策略
     - 开空（_get_sell_short_num）
     - 平空（_get_buy_short_num）
   * - MM_FixedCount
     - 按 n 分批卖出
     - 全部平空
   * - MM_FixedRisk / MM_FixedPercent / MM_FixedUnits
     - 与买入同风险预算语义（risk 为空头每股风险，即止损价 - 开空价；MM_FixedPercent / MM_FixedUnits 的基数为净权益）
     - 全部平空
   * - MM_FixedCapital / MM_FixedCapitalFunds / MM_WilliamsFixedRisk
     - 与买入同风险预算语义（不使用 risk，基数为净权益）
     - 全部平空
   * - MM_Nothing
     - 按净权益开空（净权益 / (价格 × 单位)，不使用 risk）
     - 全部平空
   * - MM_FixedCountTps
     - 0（禁止开空）
     - 全部平空

.. note::

    MM_FixedCountTps 的档位计数为多头专用，不支持开空。需要开空能力时使用上述支持开空的策略，或以 Python 自定义资金管理策略重载 _get_sell_short_num / _get_buy_short_num。

    开空需交易管理账户开启 ``support_borrow_stock=True``（融资融券模式），否则开空委托直接失败。开空数量与买入一致做手数对齐并受单笔最大交易数量限制；开空不做保证金约束，保证金（购买力）不足时由交易管理拒绝整单，不做缩量。

    空头的资金基数为账户净权益（net assets = 现金 + 持仓市值 − 空头市值 − 借券现金，Van Tharp 口径），借券卖出款不计入风险预算；多头仍以可用现金为基数（无持仓时两者相等，有持仓时更保守）。


自定义资金管理策略
--------------------

自定义资金管理策略接口：

* :py:meth:`MoneyManagerBase._buy_notify` - 【可选】接收实际买入通知，预留用于多次增减仓处理
* :py:meth:`MoneyManagerBase._sell_notify` - 【可选】接收实际卖出通知，预留用于多次增减仓处理
* :py:meth:`MoneyManagerBase._get_buy_num` - 【必须】获取指定交易对象可买入的数量
* :py:meth:`MoneyManagerBase._get_sell_num` - 【可选】获取指定交易对象可卖出的数量，如未重载，默认为卖出全部已持仓数量
* :py:meth:`MoneyManagerBase._get_sell_short_num` - 【可选】获取指定交易对象可卖空（开空）的数量，如未重载，默认为 0（禁止开空）
* :py:meth:`MoneyManagerBase._get_buy_short_num` - 【可选】获取指定交易对象买入平空的数量，如未重载，默认为全部平空
* :py:meth:`MoneyManagerBase._reset` - 【可选】重置私有属性
* :py:meth:`MoneyManagerBase._clone` - 【必须】克隆接口

资金管理策略基类
----------------

.. py:class:: MoneyManagerBase

    资金管理策略基类

    .. py:attribute:: name  名称
    .. py:attribute:: tm    设置或获取交易管理对象
    .. py:attribute:: query 设置或获取查询条件

    .. py:attribute:: support_mult_buy_sell 是否支持多次增减仓（默认 False）

        仅 Python 自定义资金管理策略可改写该属性（内建策略赋值将抛出异常），克隆时保留。为 False（默认）时，卖出与买入平空遇到非正风险（含 NaN，即价格已触及止损）直接强制全平；为 True 时交由子类的 _get_sell_num / _get_buy_short_num 自行处理。

    .. py:method:: __init__(self[, name="MoneyManagerBase])

        初始化构造函数

        :param str name: 名称

    .. py:method:: get_param(self, name)

        获取指定的参数

        .. note::

            所有的资金管理策略具有参数“auto_checkin”（bool类型，默认为False），其含义为“当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中补充存入（checkin）足够的现金。注资由系统在实际成交时一次性完成，数量查询本身不改变账户。

        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数

    .. py:method:: set_param(self, name, value)

        设置参数

        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: reset(self)

        复位操作

    .. py:method:: clone(self)

        克隆操作

    .. py:method:: get_buy_num(self, datetime, stock, price, risk, part_from)

        获取指定交易对象可买入的数量

        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 交易承担的每股风险，必须大于 0；非正值或 NaN 时被拒绝，返回 0
        :param System.Part part_from: 来源系统组件
        :return: 可买入数量
        :rtype: float

    .. py:method:: get_sell_num(self, datetime, stock, price, risk, part_from)

        获取指定交易对象可卖出的数量

        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
        :param System.Part part_from: 来源系统组件
        :return: 可卖出数量
        :rtype: float

    .. py:method:: current_buy_count(self, stock)

        当前连续买入计数

    .. py:method:: current_sell_count(self, stock)

        当前连续卖出计数

    .. py:method:: _buy_notify(self, trade_record)

        【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载

        :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录

    .. py:method:: _sell_notify(self, trade_record)

        【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载

        :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录

    .. py:method:: _get_buy_num(self, datetime, stock, price, risk, part_from)

        【重载接口】获取指定交易对象可买入的数量

        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 交易承担的每股风险，必须大于 0；非正值或 NaN 时被拒绝，返回 0
        :param System.Part part_from: 来源系统组件
        :return: 可买入数量
        :rtype: float

    .. py:method:: _get_sell_num(self, datetime, stock, price, risk, part_from)

        【重载接口】获取指定交易对象可卖出的数量。如未重载，默认为卖出全部已持仓数量。

        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
        :param System.Part part_from: 来源系统组件
        :return: 可卖出数量
        :rtype: float

    .. py:method:: _reset(self)

        【重载接口】子类复位接口，复位内部私有变量

    .. py:method:: _clone(self)

        【重载接口】子类克隆接口
