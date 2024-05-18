.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

信号指示器
==========

信号指示器负责产生买入、卖出信号。

公共参数：

    * **alternate** *(bool|True)* ：买入和卖出信号是否交替出现。单线型的信号通常通过拐点、斜率等判断信号的产生，此种情况下可能出现连续出现买入信号或连续出现卖出信号的情况，此时可通过该参数控制买入、卖出信号是否交替出现。而双线交叉型的信号通常本身买入和卖出已经是交替出现，此时该参数无效。


通用信号指示器
--------------

通常使用技术指标判断买入、卖出时，依据的是快线和慢线的交叉、或是单曲线的拐点。下面的通用信号指示器足够应付大部分的情况。

双线交叉信号指示器
^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cross(fast, slow)
    
    双线交叉指示器，当快线从下向上穿越慢线时，买入；当快线从上向下穿越慢线时，卖出。如：5日MA上穿10日MA时买入，5日MA线下穿MA10日线时卖出:: 

        SG_Cross(MA(CLOSE(), n=10), OP(MA(n=30)))

    :param Indicator fast: 快线
    :param Indicator slow: 慢线
    :return: 信号指示器
        
        
金叉信号指示器
^^^^^^^^^^^^^^^

.. py:function:: SG_CrossGold(fast, slow)

    金叉指示器，当快线从下向上穿越慢线且快线和慢线的方向都是向上时为金叉，买入；
    当快线从上向下穿越慢线且快线和慢线的方向都是向下时死叉，卖出。::
    
        SG_CrossGold(MA(CLOSE(), n=10), MA(CLOSE(), n=30))
    
    :param Indicator fast: 快线
    :param Indicator slow: 慢线
    :return: 信号指示器    
        

单线拐点信号指示器
^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Single(ind[, filter_n = 10, filter_p = 0.1])
    
    生成单线拐点信号指示器。使用《精明交易者》 [BOOK1]_ 中给出的曲线拐点算法判断曲线趋势，公式见下::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Indicator ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :return: 信号指示器
    
.. py:function:: SG_Single2(ind[, filter_n = 10, filter_p = 0.1])
    
    生成单线拐点信号指示器2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Indicator ind:
    :param int filer_n: N日周期
    :param float filter_p: 过滤器百分比
    :return: 信号指示器
   
自交叉单线拐点指示器
^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Flex(ind, slow_n)

    使用自身的EMA(slow_n)作为慢线，自身作为快线，快线向上穿越慢线买入，快线向下穿越慢线卖出。

    :param Indicator ind:
    :param int slow_n: 慢线EMA周期
    :return: 信号指示器


布尔信号指示器
^^^^^^^^^^^^^^^^

.. py:function:: SG_Bool(buy, sell)

    布尔信号指示器，使用运算结果为类似bool数组的Indicator分别作为买入、卖出指示。
    
    :param Indicator buy: 买入指示（结果Indicator中相应位置>0则代表买入）
    :param Indicator sell: 卖出指示（结果Indicator中相应位置>0则代表卖出）
    :return: 信号指示器


区间突破信号指示器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Band(ind, lower, upper)
          
    指标区间指示器, 当指标超过上轨时，买入；
    当指标低于下轨时，卖出。
    
    ::

        SG_Band(MA(C, n=10), 100, 200)
        SG_Band(CLOSE, MA(LOW), MA(HIGH))


持续买入信号指示器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_AllwaysBuy()
    
    一个特殊的SG，持续每天发出买入信号，通常配合 PF 使用


PF调仓周期买入信号指示器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cycle()
    
    一个特殊的SG，配合PF使用，以 PF 调仓周期为买入信号


自定义信号指示器
----------------

快速创建不带私有属性的自定义信号指示器

.. py:function:: crtSG(func, params={}, name='crtSG')

    快速创建自定义不带私有属性的信号指示器
    
    :param func: 信号策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义信号指示器实例
    
示例：

.. literalinclude:: ../../examples/quick_crtsg.py      

自定义的信号指示器接口：

* :py:meth:`SignalBase._calculate` - 【必须】子类计算接口
* :py:meth:`SignalBase._clone` - 【必须】克隆接口
* :py:meth:`SignalBase._reset` - 【可选】重载私有变量

示例1（不含私有变量，海龟交易策略）:

.. literalinclude:: ../../examples/Turtle_SG.py                
                
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

.. py:class:: SignalBase

    信号指示器基类
    
    .. py:attribute:: name 名称
    
    .. py:method:: __init__(self[, name="SignalBase"])
    
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
                
    .. py:method:: should_buy(self, datetime)
    
        指定时刻是否可以买入
    
        :param Datetime datetime: 指定时刻
        :rtype: bool
    
    .. py:method:: should_sell(self, datetime)
    
        指定时刻是否可以卖出
        
        :param Datetime datetime: 指定时刻
        :rtype: bool

    .. py:method:: next_time_should_buy(self)

        下一时刻是否可以买入，相当于最后时刻是否指示买入

    .. py:method:: next_time_should_sell(self)

        下一时刻是否可以卖出，相当于最后时刻是否指示卖出
    
    .. py:method:: get_buy_signal(self)
    
        获取所有买入指示日期列表
        
        :rtype: DatetimeList
    
    .. py:method:: get_sell_signal(self)
    
        获取所有卖出指示日期列表
        
        :rtype: DatetimeList
    
    .. py:method:: _add_buy_signal(self, datetime)
    
        加入买入信号，在_calculate中调用
        
        :param Datetime datetime: 指示买入的日期
    
    .. py:method:: _add_sell_signal(self, datetime)
    
        加入卖出信号，在_calculate中调用

        :param Datetime datetime: 指示卖出的日期
        
    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作
    
    .. py:method:: _calculate(self, kdata)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口