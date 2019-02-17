.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

盈利目标策略
============

内建盈利目标策略
----------------

.. py:function:: PG_FixedPercent([p = 0.2])

    固定百分比盈利目标，目标价格 = 买入价格 * (1 + p)
    
    :param float p: 百分比
    :return: 盈利目标策略实例
    
.. py:function:: PG_FixedHoldDays([days=5])

    固定持仓天数盈利目标策略
    
    :param int days: 允许持仓天数（按交易日算）,默认5天
    :return: 盈利目标策略实例
    
.. py:function:: PG_NoGoal()

    无盈利目标策略，通常为了进行测试或对比。
    
    :return: 盈利目标策略实例

    
自定义盈利目标策略
------------------    

自定义盈利目标策略接口：

* :py:meth:`ProfitGoalBase.getGoal` - 【必须】获取目标价格
* :py:meth:`ProfitGoalBase._calculate` - 【必须】子类计算接口
* :py:meth:`ProfitGoalBase._clone` - 【必须】克隆接口
* :py:meth:`ProfitGoalBase._reset` - 【可选】重载私有变量
* :py:meth:`ProfitGoalBase.buyNotify` - 【可选】接收实际买入通知，预留用于多次增减仓处理
* :py:meth:`ProfitGoalBase.sellNotify` - 【可选】接收实际卖出通知，预留用于多次增减仓处理
    

盈利目标策略基类
----------------

.. py:class:: ProfitGoalBase

    盈利目标策略基类
    
    .. py:attribute:: name 名称
    
    .. py:method:: __init__(self[, name="ProfitGoalBase"])
    
        初始化构造函数
        
        :param str name: 名称
        
    .. py:method:: getParam(self, name)

        获取指定的参数
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: setParam(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型
        
    .. py:method:: setTO(self, k)
    
        :param KData k: 设置交易对象
        
    .. py:method:: getTO(self)
    
        :return: 交易对象
        :rtype: KData
        
    .. py:method:: setTM(self, tm)
    
        :param TradeManager tm: 设置交易管理账户
        
    .. py:method:: getTM(self)
    
        获取交易管理账户
        
        :rtype: TradeManager

    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: getGoal(self, datetime, price)
    
        【重载接口】获取盈利目标价格，返回constant.null_price时，表示未限定目标；返回0意味着需要卖出
        
        :param Datetime datetime: 买入时间
        :param float price: 买入价格
        :return: 目标价格
        :rtype: float
        
    .. py:method:: buyNotify(self, trade_record)
    
        【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载
        
        :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录
        
    .. py:method:: sellNotify(self, trade_record)
    
        【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载
        
        :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录
         
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口        
     