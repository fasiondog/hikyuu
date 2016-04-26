.. py:currentmodule:: trade_sys
.. highlightlang:: python

信号指示器
==========

信号指示器负责产生买入、卖出信号。


通用信号指示器
--------------

通常使用技术指标判断买入、卖出时，依据的是快线和慢线的交叉、或是单曲线的拐点。下面的通用信号指示器足够应付大部分的情况。

双线交叉信号指示器
^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cross(fast, slow[, kpart = "CLOSE"])
    
    双线交叉指示器，当快线从下向上穿越慢线时，买入；当快线从上向下穿越慢线时，卖出。如：5日MA上穿10日MA时买入，5日MA线下穿MA10日线时卖出:: 

        SG_Cross(OP(MA(n=10)), OP(MA(n=30)))

    :param Operand fast: 快线
    :param Operand slow: 慢线
    :param string kpart: OPEN|HIGH|LOW|CLOSE|AMO|VOL|KDATA
    :return: 信号指示器
        

单线拐点信号指示器
^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Single(ind[, filter_n = 10, filter_p = 0.1, kpart='CLOSE'])
    
    生成单线拐点信号指示器。使用《精明交易者》 [BOOK1]_ 中给出的曲线拐点算法判断曲线趋势，公式见下::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Operand ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
    
.. py:function:: SG_Single2(ind[, filter_n = 10, filter_p = 0.1, kpart='CLOSE'])
    
    生成单线拐点信号指示器2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Operand ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器
   
自交叉单线拐点指示器
^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Flex(ind, slow_n[, kpart = 'CLOSE'])

    使用自身的EMA(slow_n)作为慢线，自身作为快线，快线向上穿越慢线买入，快线向下穿越慢线卖出。

    :param Operand ind:
    :param int slow_n: 慢线EMA周期
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 信号指示器

自定义信号指示器
----------------

自定义信号指示器，必须实现 :py:meth:`SignalBase._clone` 和 :py:meth:`SignalBase._calculate`  方法（如示例1）。如果含有私有属性，还需实现 :py:meth:`SignalBase._reset` 方法（如示例2）。

示例1（不含私有变量，海龟交易策略）:

.. literalinclude:: ../../../examples/Turtle_SG.py                
                
示例2（含私有属性）:

::

    class SignalPython(SignalBase):
        def __init__(self):
            super(SignalPython, self).__init__("SignalPython")
            self._x = 0 #私有属性
            self.setParam("test", 30)
        
        def _reset(self):
            self._x = 0
                
        def _clone(self):
            p = SignalPython()
            p._x = self._x
            return p
        
        def _calculate(self):
            self._addBuySignal(Datetime(201201210000))
            self._addSellSignal(Datetime(201201300000))


信号指示器基类
--------------

自定义的信号指示器，应实现 :py:meth:`SignalBase._clone`, :py:meth:`SignalBase._reset`, :py:meth:`SignalBase._calculate` 接口。

.. py:class:: SignalBase([name])

    信号指示器基类
    
    .. py:attribute:: name
    
        名称
        
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
        
    .. py:method:: shouldBuy(datetime)
    
        指定时刻是否可以买入
    
        :param Datetime datetime: 指定时刻
        :return: 是否可买入
        :rtype: bool(True | False)
    
    .. py:method:: shouldSell(datetime)
    
        指定时刻是否可以卖出
        
        :param Datetime datetime: 指定时刻
        :return: 是否可卖出
        :rtype: bool(True | False)
    
    .. py:method:: getBuySignal()
    
        获取所有买入指示日期列表
        
        :return: 指示买入日期列表
        :rtype: DatetimeList
    
    .. py:method:: getSellSignal()
    
        获取所有卖出指示日期列表
        
        :return: 指示卖出日期列表
        :rtype: DatetimeList
    
    .. py:method:: _addBuySignal(datetime)
    
        加入买入信号，在_calculate中调用
        
        :param Datetime datetime: 指示买入的日期
    
    .. py:method:: _addSellSignal(datetime)
    
        加入卖出信号，在_calculate中调用

        :param Datetime datetime: 指示卖出的日期
        
    .. py:method:: reset()
    
        复位操作
    
    .. py:method:: clone()
    
        克隆操作
    
    .. py:method:: _calculate()
    
        子类计算接口
    
    .. py:method:: _reset()
    
        子类复位接口，复位内部私有变量
    
    .. py:method:: _clone()
    
        子类克隆接口