.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

止损/止赢策略
=============

.. Note::

    Hikyuu中将止损和止盈分别作为交易系统的两个策略组件。两者之间在概念和执行上有所区别。比如，一般系统通常在使用跟随性的指标曲线作为止盈退出时，经常会发生滞后的情况，原本希望收盘价低于指标时卖出止盈，但实际上指标和收盘价同时都在下跌，这样实际的退出发生在收盘价向下穿越指标线时，这样造成滞后反映，另外，如果在盘中实时跟踪，由于收盘价不停的变动，止损的指标线也会发生变动，这样会出现噪音误判，导致一般系统里实盘和回测的结果出现偏差。Hikyuu里，当前Bar里止损/止盈都是不变的固定是上一时刻的值，同时，Hikyuu里系统是保证止盈始终单调递增的！比如某个指标值，前天值为11，昨天的值为9，今天的收盘价10，那么这个指标作为止损部件（今日收盘价10大于止损价9），不会触发退出，而作为止盈部件，系统则会发出卖出指示，因为当前的收盘价已经低于11。
    

常用止损/止赢策略
-----------------

止损是指买入后，价格的走势和预期相反，当价格低于某一水平时卖出，防止进一步的损失。
止赢是在买入后，价格符合预期走势，当价格回落至某一水平时卖出，获得足够的收益。
进行交易时，即可使用相同的止损和止赢策略，也可使用不同的止损和止赢策略，如使用固定百分比3%作为止损，使用吊灯安全线作为止赢。

固定百分比止损
^^^^^^^^^^^^^^

.. py:function:: ST_FixedPercent([p=0.03])

    固定百分比止损策略，即当价格低于买入价格的某一百分比时止损
    
    :param float p: 百分比(0,1]
    :return: 止损/止赢策略实例

技术指标止损
^^^^^^^^^^^^

.. py:function:: ST_Indicator(op[, kpart="CLOSE"])

    使用技术指标作为止损价。如使用10日EMA作为止损：::
    
        ST_Indicator(EMA(CLOSE(), n=10))

    :param Indicator ind: 指标公式
    :return: 止损/止赢策略实例

亚历山大.艾尔德安全地带止损
^^^^^^^^^^^^^^^^^^^^^^^^^^^^    

.. py:function:: ST_Saftyloss([n1=10, n2=3, p=2.0])

    参见《走进我的交易室》（2007年 地震出版社） 亚历山大.艾尔德(Alexander Elder) P202
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，
    得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日
    最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的
    上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param int n1: 计算平均噪音的回溯时间窗口，默认为10天
    :param int n2: 对初步止损线去n2日内的最高值，默认为3
    :param double p: 噪音系数，默认为2
    :return: 止损/止赢策略实例
    

自定义止损/止赢策略
-------------------

自定义止损/止赢策略接口：

* :py:meth:`SignalBase._calculate` - 【必须】子类计算接口
* :py:meth:`SignalBase._clone` - 【必须】克隆接口
* :py:meth:`SignalBase._reset` - 【可选】重载私有变量

止损/止赢策略基类
-----------------

.. py:class:: StoplossBase

    止损/止赢算法基类
    
    .. py:attribute:: name 名称
    .. py:attribute:: tm 设置或获取交易管理实例
    .. py:attribute:: to 设置或获取交易对象
    
    .. py:method:: __init__(self[, name="StoplossBase"])
    
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

    .. py:method:: get_price(self, datetime, price)
    
        【重载接口】获取本次预期交易（买入）时的计划止损价格，如果不存在止损价，则返回0。用于系统在交易执行前向止损策略模块查询本次交易的计划止损价。
        
        .. note::
            一般情况下，止损/止赢的算法可以互换，但止损的getPrice可以传入计划交易的价格，比如以买入价格的30%做为止损。而止赢则不考虑传入的price参数，即认为price为0.0。实际上，即使止损也不建议使用price参数，如可以使用前日最低价的30%作为止损，则不需要考虑price参数。
        
        :param Datetime datetime: 交易时间
        :param float price: 计划买入的价格
        :return: 止损价格
        :rtype: float
        
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口