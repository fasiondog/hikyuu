.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

盈利目标策略|PG
================

内建盈利目标策略
----------------

.. py:function:: PG_FixedPercent([p = 0.2])

    固定百分比盈利目标，目标价格 = 平均成本 × (1 + p)（空头为 × (1 - p)），并按标的价格精度取整；尚无持仓时以当前价格作为成本
    
    :param float p: 百分比
    :return: 盈利目标策略实例
    
.. py:function:: PG_FixedHoldDays([days=5])

    固定持仓天数盈利目标策略。持有多头或空头仓位达到指定交易日数后，在下一根 K 线强制平仓（与价格无关）；交易日按标的自身 K 线中的交易日计数。使用前必须先 setTO，否则无交易日列表、规则不生效
    
    :param int days: 允许持仓天数（按交易日算）,默认5天
    :return: 盈利目标策略实例
    
.. py:function:: PG_NoGoal()

    无盈利目标策略，通常为了进行测试或对比。
    
    :return: 盈利目标策略实例

    
返回值语义与触发方向
----------------------

``get_goal`` / ``_get_goal`` 与 ``get_short_goal`` / ``_get_short_goal`` 的返回值含义：

.. list-table::
    :header-rows: 1
    :widths: 30 70

    * - 返回值
      - 含义
    * - ``constant.null_price``
      - 未设置目标，不触发
    * - 正有限值
      - 目标价
    * - ``0``
      - 多头：需要卖出（市价强平）；空头：无效果（``price <= 0`` 永不成立）
    * - 负值、``±inf``
      - 非法，基类钳制为 ``constant.null_price``（视为未设置目标，不触发、不告警）

触发方向：

.. list-table::
    :header-rows: 1
    :widths: 25 75

    * - 方向
      - 触发条件与动作
    * - 多头
      - 当前价 >= 目标价时卖出（交易记录 ``from`` 为 ``PART_PROFITGOAL``）
    * - 空头
      - 当前价 <= 目标价时买入平仓（交易记录 ``from`` 为 ``PART_PROFITGOAL``）

.. note::
    ``PG_FixedHoldDays`` 的到期强平通过返回值哨兵实现（多头返回 0，空头返回当前价），此类强平交易记录中的 ``goalPrice`` 不具目标价含义。

自定义盈利目标策略
------------------    

自定义盈利目标策略接口：

* :py:meth:`ProfitGoalBase._get_goal` - 【必须】获取目标价格
* :py:meth:`ProfitGoalBase._get_short_goal` - 【可选】获取空头目标价格，返回constant.null_price表示未设置空头目标
* :py:meth:`ProfitGoalBase._calculate` - 【可选】子类计算接口
* :py:meth:`ProfitGoalBase._clone` - 【必须】克隆接口
* :py:meth:`ProfitGoalBase._reset` - 【可选】重载私有变量
* :py:meth:`ProfitGoalBase.buy_notify` - 【可选】接收实际买入通知，预留用于多次增减仓处理
* :py:meth:`ProfitGoalBase.sell_notify` - 【可选】接收实际卖出通知，预留用于多次增减仓处理
    

盈利目标策略基类
----------------

.. py:class:: ProfitGoalBase

    盈利目标策略基类
    
    .. py:attribute:: name 名称
    .. py:attribute:: to 设置或获取交易对象
    .. py:attribute:: tm 设置或获取交易管理账户
    
    .. py:method:: __init__(self[, name="ProfitGoalBase"])
    
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
        
    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: get_goal(self, datetime, price)
    
        获取盈利目标价格，返回constant.null_price时，表示未限定目标；返回0意味着需要卖出
        
        :param Datetime datetime: 当前时间
        :param float price: 当前价格
        :return: 目标价格
        :rtype: float
        
    .. py:method:: get_short_goal(self, datetime, price)
    
        获取空头盈利目标价格，返回constant.null_price时，表示未设置空头目标
        
        :param Datetime datetime: 当前时间
        :param float price: 当前价格
        :return: 空头目标价格
        :rtype: float
        
    .. py:method:: _get_goal(self, datetime, price)
    
        【重载接口】获取盈利目标价格，返回constant.null_price时，表示未限定目标；返回0意味着需要卖出
        
        :param Datetime datetime: 当前时间
        :param float price: 当前价格
        :return: 目标价格
        :rtype: float
        
    .. py:method:: _get_short_goal(self, datetime, price)
    
        【重载接口】获取空头盈利目标价格，返回constant.null_price时，表示未设置空头目标；当前价格 <= 目标价时买入平仓；空头成本按当前空头持仓计算
        
        :param Datetime datetime: 当前时间
        :param float price: 当前价格
        :return: 空头目标价格
        :rtype: float
        
    .. py:method:: buy_notify(self, trade_record)
    
        【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载
        
        :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录
        
    .. py:method:: sell_notify(self, trade_record)
    
        【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载
        
        :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录
         
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口        
     