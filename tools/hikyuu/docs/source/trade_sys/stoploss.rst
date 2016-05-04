.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

止损/止赢策略
=============

常用止损/止赢策略
-----------------

止损是指买入后，价格的走势和预期相反，当价格低于某一水平时卖出，防止进一步的损失。
止赢是在买入后，价格符合预期走势，当价格回落至某一水平时卖出，获得足够的收益。
进行交易时，即可使用相同的止损和止赢策略，也可使用不同的止损和止赢策略，如使用固定百分比3%作为止损，使用吊灯安全线作为止赢。

固定百分比止损
^^^^^^^^^^^^^^

.. py:function:: ST_FixedPercent([p = 0.03])

    固定百分比止损策略，即当价格低于买入价格的某一百分比时止损
    
    :param double p: 百分比(0,1]
    :return: 止损/止赢策略实例

技术指标止损
^^^^^^^^^^^^

.. py:function:: ST_Indicator(op[, kpart = "CLOSE"])

    使用技术指标作为止损价。如使用10日EMA作为止损：::
    
        ST_Indicator(OP(EMA(n=10)))

    :param Operand op:
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL


自定义止损/止赢策略
-------------------

自定义止损/止赢策略接口：

* :py:meth:`SignalBase._calculate` - 【必须】子类计算接口
* :py:meth:`SignalBase._clone` - 【必须】克隆接口
* :py:meth:`SignalBase._reset` - 【可选】重载私有变量

止损/止赢策略基类
-----------------

.. py:class:: StoplossBase([name])

    止损/止赢算法基类
    
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
        :type value: int | bool | double | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: setTM(tm)
        
        设置交易管理实例
        
        :param TradeManager tm: 交易管理实例
        
    .. py:method:: setTO(k)
    
        :param KData k: 设置交易对象
        
    .. py:method:: getTO()
    
        :return: 交易对象
        :rtype: KData
   
    .. py:method:: reset()
    
        复位操作
    
    .. py:method:: clone()
    
        克隆操作

    .. py:method:: getPrice(datetime, price)
    
        【重载接口】获取本次预期交易（买入）时的计划止损价格，如果不存在止损价，则返回0。用于系统在交易执行前向止损策略模块查询本次交易的计划止损价。
        
        .. note::
            一般情况下，止损/止赢的算法可以互换，但止损的getPrice可以传入计划交易的价格，比如以买入价格的30%做为止损。而止赢则不考虑传入的price参数，即认为price为0.0。实际上，即使止损也不建议使用price参数，如可以使用前日最低价的30%作为止损，则不需要考虑price参数。
        
        :param Datetime datetime: 交易时间
        :param double price: 计划买入的价格
        
    .. py:method:: _calculate()
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset()
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone()
    
        【重载接口】子类克隆接口