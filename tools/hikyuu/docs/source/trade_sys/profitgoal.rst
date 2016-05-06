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
    
.. py:function:: PG_NoGoal()

    无盈利目标策略，通常为了显示测试。
    
    :return: 盈利目标策略实例

    
自定义盈利目标策略
------------------    

自定义盈利目标策略接口：

* :py:meth:`ProfitGoalBase.getGoal` - 【必须】获取目标价格
* :py:meth:`ProfitGoalBase._calculate` - 【必须】子类计算接口
* :py:meth:`ProfitGoalBase._clone` - 【必须】克隆接口
* :py:meth:`ProfitGoalBase._reset` - 【可选】重载私有变量
    

盈利目标策略基类
----------------

.. py:class:: ProfitGoalBase([name])

    盈利目标策略基类
    
    .. py:attribute:: name 名称
        
    .. py:method:: getParam(name)

        获取指定的参数
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: setParam(name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型
        
    .. py:method:: setTO(k)
    
        :param KData k: 设置交易对象
        
    .. py:method:: getTO()
    
        :return: 交易对象
        :rtype: KData
        
    .. py:method:: setTM(tm)
    
        :param TradeManager tm: 设置交易管理账户
        
    .. py:method:: getTM()
    
        获取交易管理账户
        
        :return: 交易管理账户
        
    .. py:method:: getGoal(datetime, price)
    
        【重载接口】获取盈利目标价格
        
        :param Datetime datetime: 买入时间
        :param float price: 买入价格
        :return: 目标价格
        :rtype: float

    .. py:method:: reset()
    
        复位操作
    
    .. py:method:: clone()
    
        克隆操作        
        
    .. py:method:: _calculate()
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset()
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone()
    
        【重载接口】子类克隆接口        
     