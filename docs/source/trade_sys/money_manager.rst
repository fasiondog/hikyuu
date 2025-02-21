.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

资金管理策略
============

公共参数：

    * **auto-checkin=False** *(bool)* : 当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中补充存入（checkin）足够的现金。
    * **max-stock=20000** *(int)* : 最大持有的证券种类数量（即持有几只股票，而非各个股票的持仓数）
    * **disable_ev_force_clean_position=False** *(bool)* : 禁用市场环境失效时强制清仓
    * **disable_cn_force_clean_position=False** *(bool)* : 禁用系统有效条件失效时强制清仓


内建资金管理策略
----------------

不做资金管理策略
^^^^^^^^^^^^^^^^^^

.. py:function:: MM_Nothing()

    特殊的资金管理策略，相当于不做资金管理，有多少钱买多少。


固定交易数量资金管理策略
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCount([n = 100])

    固定交易数量资金管理策略。每次买入固定的数量。
    
    :param float n: 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
    :return: 资金管理策略实例

.. py:function:: MM_FixedCountTps([buy_counts, sell_counts])
          
    连续买入/卖出固定数量资金管理策略。
    
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

    固定资金管理策略, 即控制每次买入投入的总资金。买入数量 = 当前现金 / capital

    :param float capital: 固定资本单位
    :return: 资金管理策略实例

.. py:function:: MM_FixedCapitalFunds([capital = 10000.0])

    固定资本管理策略。买入数量 = 当前总资产 / capital

    :param float capital: 固定资本单位
    :return: 资金管理策略实例    


固定单位资金风险管理策略
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedUnits([n = 33])

    固定单位资金管理策略。公式: 买入数量 = 当前现金 / n / 当前风险risk

    :param int n: n个资金单位
    :return: 资金管理策略实例
    

威廉斯固定风险资金管理策略
^^^^^^^^^^^^^^^^^^^^^^^^^^  

.. py:function:: MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    威廉斯固定风险资金管理策略，买入数量 =（账户余额 × 风险百分比p）÷ 最大损失(max_loss)
    
    :param float p: 风险百分比
    :param float max_loss: 最大损失
    :return: 资金管理策略实例

    
固定百分比资金管理策略
^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedPercent([p = 0.03])

    固定百分比风险模型。公式：P（头寸规模）＝ 账户余额 * 百分比 / R（每股的交易风险）。[BOOK3]_, [BOOK4]_ .
    
    :param float p: 百分比
    :return: 资金管理策略实例
    

固定波幅资金管理策略
^^^^^^^^^^^^^^^^^^^^


    

自定义资金管理策略
--------------------

自定义资金管理策略接口：

* :py:meth:`MoneyManagerBase.buyNotify` - 【可选】接收实际买入通知，预留用于多次增减仓处理
* :py:meth:`MoneyManagerBase.sellNotify` - 【可选】接收实际卖出通知，预留用于多次增减仓处理
* :py:meth:`MoneyManagerBase._getBuyNumber` - 【必须】获取指定交易对象可买入的数量
* :py:meth:`MoneyManagerBase._getSellNumber` - 【可选】获取指定交易对象可卖出的数量，如未重载，默认为卖出全部已持仓数量
* :py:meth:`MoneyManagerBase._reset` - 【可选】重置私有属性
* :py:meth:`MoneyManagerBase._clone` - 【必须】克隆接口

资金管理策略基类
----------------

.. py:class:: MoneyManagerBase

    资金管理策略基类
    
    .. py:attribute:: name  名称
    .. py:attribute:: tm    设置或获取交易管理对象
    .. py:attribute:: query 设置或获取查询条件
    
    .. py:method:: __init__(self[, name="MoneyManagerBase])
    
        初始化构造函数
        
        :param str name: 名称
        
    .. py:method:: get_param(self, name)

        获取指定的参数
        
        .. note::

            所有的资金管理策略具有参数“auto-checkin”（bool类型，默认为False），其含义为“当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中补充存入（checkin）足够的现金。
    
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
        :param float risk: 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
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
        :param float risk: 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
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