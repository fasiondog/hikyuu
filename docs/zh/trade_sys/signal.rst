.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

信号指示器|SG
=============

信号指示器负责产生买入、卖出信号。

公共参数：

    * **alternate** *(bool|True)* ：买入和卖出信号是否交替出现。单线型的信号通常通过拐点、斜率等判断信号的产生，此种情况下可能出现连续出现买入信号或连续出现卖出信号的情况，此时可通过该参数控制买入、卖出信号是否交替出现。而双线交叉型的信号通常本身买入和卖出已经是交替出现，此时该参数无效。
    * **cycle** *(bool|False)* : 标记该信号依赖 MultiSystem 注入的调仓日（如 SG_Cycle），不再改变计算范围：信号始终按完整 K 线数据一次性计算，回测期内结果保持不变。周期信号是计划性信号，``cycle=true`` 与 ``alternate=true`` 互斥，设置 cycle 时 alternate 会被强制为 false（并给出告警）；组合运算中只要包含周期子信号，组合结果同样继承 cycle 标记与 alternate=False，避免后续调仓日的买入被交替规则丢弃。模式 A/B 在运行前注入整段调仓日，模式 C 在子系统入选日声明（已运行的系统不再重复声明，沿用 legacy Portfolio 语义）。仅 ``cycle=true`` 的信号会消费这些调仓日，普通信号不受影响、也不会因声明而重算。注意：周期买入是子系统的“计划入场/再入场”信号（首次建仓、自身平仓后、被 SE 取消选中清仓后再入选），不是组合层调仓动作；组合层调仓由 SE/AF 完成。已持仓时周期买入能否加仓取决于 MM（如 MM_Nothing 在现金用尽后不会再成交）。**组合限制**：模式 C 下 cycle 信号不可与 ``SE_Signal`` 组合——``SE_Signal`` 在 run 前一次性快照子系统已算好的买入信号，而模式 C 要到入选才声明周期日，快照恒为空，因此永远选不中（不进运行池、不成交）；按固定标的/调仓日入选的选择器（如 ``SE_Fixed``）不受影响，模式 A/B 先注入整段调仓日再快照，可与 ``SE_Signal`` 组合。另：模式 C 只在入选时声明周期日，同一周期内被止损平仓不会自动重开，需等 SE 取消其选中再重新入选。
    * **support_borrow_stock** *(bool|False)* : 支持发出空头信号


通用信号指示器
--------------

通常使用技术指标判断买入、卖出时，依据的是快线和慢线的交叉、或是单曲线的拐点。下面的通用信号指示器足够应付大部分的情况。

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>代码</th>
                <th>名称</th>
                <th>描述</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td><a href="#target-section">SG_Cross</a></td>
                <td>双线交叉指示器</td>
                <td>当快线从下向上穿越慢线时，买入；<br>当快线从上向下穿越慢线时，卖出。</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_CrossGold</a></td>
                <td>金叉指示器</td>
                <td>当快线从下向上穿越慢线且快线和慢线的方向都是向上时为金叉，买入；<br>当快线从上向下穿越慢线且快线和慢线的方向都是向下时死叉，卖出。</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Single</a></td>
                <td>单线拐点信号指示器</td>
                <td>生成单线拐点信号指示器。使用《精明交易者》中给出的曲线拐点算法判断曲线趋势</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Single2</a></td>
                <td>单线拐点信号指示器2</td>
                <td>生成单线拐点信号指示器。使用《精明交易者》中给出的曲线拐点算法判断曲线趋势</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Flex</a></td>
                <td>自交叉单线拐点指示器</td>
                <td>使用自身的EMA(slow_n)作为慢线，自身作为快线。<br>快线向上穿越慢线买入，<br>快线向下穿越慢线卖出。</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Bool</a></td>
                <td>布尔信号指示器</td>
                <td>使用运算结果为类似bool数组的Indicator分别作为买入、卖出指示。</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_OneSide</a></td>
                <td>单边信号指示器</td>
                <td>根据输入指标构建单边信号（单纯的只包含买入或卖出信号），<br>如果指标值大于0，则加入信号</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Buy</a></td>
                <td>单边买入信号指示器</td>
                <td>SG_OneSide 简化模式</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Sell</a></td>
                <td>单边卖出信号指示器</td>
                <td>SG_OneSide简化模式</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Band</a></td>
                <td>区间突破信号指示器</td>
                <td>指标区间指示器, 当指标超过上轨时，买入；<br>当指标低于下轨时，卖出。</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_AllwaysBuy</a></td>
                <td>持续买入信号指示器</td>
                <td>一个特殊的SG，持续每天发出买入信号，通常配合 PF 使用</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Cycle</a></td>
                <td>PF调仓周期买入信号指示器</td>
                <td>一个特殊的SG，在 MultiSystem 运行前注入的调仓日产生买入信号</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Manual</a></td>
                <td>手工信号指示器</td>
                <td>信号只能手工加入，用于测试或外部信号导入</td>
            </tr>
            <tr>
                <td>SG_Add<br>SG_Mul<br>SG_Sub<br>SG_Div<br>SG_And<br>SG_Or</td>
                <td>SG运算辅助</td>
                <td>由于 SG 的 alternate 默认为 True, 在使用如  "sg1 + sg2 + sg3" 的形式时，容易忽略 sg1 + sg2 的 alternate 属性<br>建议使用: SG_Add(sg1, sg2, False) + sg3 来避免 alternate 的问题<br>加减乘除保留信号数值，与/或只保留方向（见下文信号运算）</td>
            </tr>        
        </tbody>
    </table>
    <p></p>


双线交叉信号指示器
^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cross(fast, slow)
    
    双线交叉指示器，当快线从下向上穿越慢线时，买入；当快线从上向下穿越慢线时，卖出。如：5日MA上穿10日MA时买入，5日MA线下穿MA10日线时卖出:: 

        SG_Cross(MA(CLOSE(), n=10), MA(CLOSE(), n=30))

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

.. py:function:: SG_Single(ind[, filter_n = 20, filter_p = 0.1])
    
    生成单线拐点信号指示器。使用《精明交易者》 [BOOK1]_ 中给出的曲线拐点算法判断曲线趋势，公式见下::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Indicator ind:
    :param int filter_n: N日周期
    :param float filter_p: 过滤器百分比
    :return: 信号指示器
    
.. py:function:: SG_Single2(ind[, filter_n = 20, filter_p = 0.1])
    
    生成单线拐点信号指示器2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Indicator ind:
    :param int filter_n: N日周期
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

.. py:function:: SG_Bool(buy, sell[, alternate=True])

    布尔信号指示器，使用运算结果为类似bool数组的Indicator分别作为买入、卖出指示。
    
    :param Indicator buy: 买入指示（结果Indicator中相应位置>0则代表买入）
    :param Indicator sell: 卖出指示（结果Indicator中相应位置>0则代表卖出）
    :param bool alternate: 是否交替买入卖出，默认为True
    :return: 信号指示器


单边信号指示器
^^^^^^^^^^^^^^^^

.. py:function:: SG_OneSide(ind, is_buy)
          
    根据输入指标构建单边信号（单纯的只包含买入或卖出信号），如果指标值大于0，则加入信号
    
    :param Indicator ind: 输入指标
    :param bool is_buy: 构建的是买入信号，否则为卖出信号


.. py:function:: SG_Buy(ind)

    单边买入信号, SG_OneSide 简化

    :param Indicator ind: 输入指标
    :return: 信号指示器


.. py:function:: SG_Sell(ind)

    单边卖出信号, SG_OneSide 简化

    :param Indicator ind: 输入指标
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
    
    一个特殊的SG，在 MultiSystem 运行前注入的调仓日产生买入信号；未注入调仓日时保持静默。
    Python 自定义周期信号可在 ``_calculate`` 中通过 ``get_cycle_dates()`` 读取注入的调仓日。
    周期信号通常用于按日程入选的选择器（如 ``SE_Fixed``），与 ``SE_Signal`` 的组合限制见上文 **cycle** 公共参数说明。
    调仓日按驱动轴声明：若该日该标的无 K 线（停牌，或 ``axis_mode=calendar`` 下市场日历有该日而标的无 bar），
    买入不会成交，也不会顺延到复牌后的首个 bar。


手工信号指示器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Manual()

    信号只能手工加入的信号指示器，用于测试或外部信号导入。其 ``_calculate`` 为空：预置信号是该指示器的
    输入，直接作为系统信号或作为运算的子信号时均被保留，设置交易对象或重置不会清空。


信号运算
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SG 支持 ``+ - * /``（``SG_Add`` / ``SG_Sub`` / ``SG_Mul`` / ``SG_Div``）和 ``& |``（``SG_And`` / ``SG_Or``）两类运算。

- 算术运算保留信号的数值幅度：``sg * 2`` 使每日信号值翻倍，``sg1 + sg2`` 同日按净额合并。
- 逻辑运算只保留方向，幅度被丢弃：命中时买入信号恒为 ``1.0``、卖出信号恒为 ``-1.0``（如 ``(sg1 * 3) & sg2`` 得 1.0 而不是 3.0）；需保留幅度请用算术运算。``|`` 在同一买卖冲突日卖出优先。


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
        
        def _calculate(self, k):
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

    .. py:method:: get_buy_value(self, datetime)

        获取指定时刻的买入信号数值；数值小于等于 0 表示无买入信号

        :param Datetime datetime: 指定时刻
        :rtype: float

    .. py:method:: get_sell_value(self, datetime)

        获取指定时刻的卖出信号数值；数值大于等于 0 表示无卖出信号

        :param Datetime datetime: 指定时刻
        :rtype: float

    .. py:method:: get_value(self, datetime)

        获取指定时刻的净信号值（买入信号值 + 卖出信号值）：正数表示买入意向，负数表示卖出意向，
        约等于 0 表示无信号

        :param Datetime datetime: 指定时刻
        :rtype: float

    .. py:method:: get_cycle_dates(self)

        获取 MultiSystem 注入的调仓日列表（已排序去重）。自定义周期信号可在 ``_calculate``
        中读取，用于确定应在哪些日期发出计划信号。

        :rtype: DatetimeList

    .. py:method:: set_cycle_dates(self, dates)

        声明周期信号消费的调仓日，通常由 MultiSystem 在运行前注入，普通信号忽略。日期会自动
        排序去重；已设置交易对象时立即重算。

        :param dates: Datetime 序列（list / tuple / DatetimeList 均可）
    
    .. py:method:: _add_signal(self, datetime, value)

        加入指定数值的信号，在 ``_calculate`` 中调用。

        同一日期的多次写入按数值合并为净额：净额为正时该日仅保留买入信号，为负时仅保留卖出信号，
        同日相反方向的已有记录会被移除；净额约为 0 时该日不保留信号（已有记录会被清除）。
        ``_add_buy_signal`` 与 ``_add_sell_signal`` 走同一合并逻辑。

        :param Datetime datetime: 信号日期
        :param float value: 信号值，正数表示买入意向，负数表示卖出意向

    .. py:method:: _add_buy_signal(self, datetime[, value=1.0])
    
        加入买入信号，在 ``_calculate`` 中调用。value 必须大于 0。

        该值会与同日已有信号合并为净额：净额保持为正时记录买入信号；转为负时该日不写入买入信号，
        已有卖出记录被更新为净额；净额约为 0 时该日记录一并清除。
        
        :param Datetime datetime: 指示买入的日期
        :param float value: 信号值，默认为 1.0
    
    .. py:method:: _add_sell_signal(self, datetime[, value=-1.0])
    
        加入卖出信号，在 ``_calculate`` 中调用。value 必须小于 0。

        该值会与同日已有信号合并为净额：净额保持为负时记录卖出信号；转为正时该日不写入卖出信号，
        已有买入记录被更新为净额；净额约为 0 时该日记录一并清除。

        :param Datetime datetime: 指示卖出的日期
        :param float value: 信号值，默认为 -1.0
        
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