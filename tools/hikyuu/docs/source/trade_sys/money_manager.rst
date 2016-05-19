.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

资金管理策略
============

.. note::

    所有的资金管理策略具有参数“auto-checkin”（bool类型，默认为False），其含义为：当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中补充存入（checkin）足够的现金。


内建资金管理策略
----------------

固定交易数量资金管理策略
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCount([n = 100])

    固定交易数量资金管理策略。每次买入固定的数量。
    
    :param int n: 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
    :return: 资金管理策略实例


固定风险资金管理策略
^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedRisk([risk = 1000.00])

    :param float risk: 固定风险
    :return: 资金管理策略实例
    

固定资本资金管理策略
^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCapital([capital = 10000.0])

    :param float capital: 固定资本单位
    :return: 资金管理策略实例


固定比例资金管理策略
^^^^^^^^^^^^^^^^^^^^


固定单位资金管理策略
^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedUnits([n = 33])

    :param int n: n个资金单位
    :return: 资金管理策略实例
    

威廉斯固定风险资金管理策略
^^^^^^^^^^^^^^^^^^^^^^^^^^  

    
    
固定百分比资金管理策略
^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedPercent([p = 0.03])

    固定百分比风险模型。公式：P（头寸规模）＝ 账户余额 * 百分比 / R（每股的交易风险）。[BOOK3]_, [BOOK4]_ .
    
    :param float p: 百分比
    :return: 资金管理策略实例
    

固定波幅资金管理策略
^^^^^^^^^^^^^^^^^^^^


    

自定义资金管理策略
------------------

自定义资金管理策略接口：

* :py:meth:`MoneyManagerBase.buyNotify` - 【可选】接收实际买入通知，预留用于多次增减仓处理
* :py:meth:`MoneyManagerBase.sellNotify` - 【可选】接收实际卖出通知，预留用于多次增减仓处理
* :py:meth:`MoneyManagerBase._getBuyNumber` - 【必须】获取指定交易对象可买入的数量
* :py:meth:`MoneyManagerBase._getSellNumber` - 【可选】获取指定交易对象可卖出的数量，如未重载，默认为卖出全部已持仓数量
* :py:meth:`MoneyManagerBase._reset` - 【可选】重置私有属性
* :py:meth:`MoneyManagerBase._clone` - 【必须】克隆接口

资金管理策略基类
----------------

.. py:class:: MoneyManagerBase([name])

    资金管理策略基类
    
    .. py:attribute:: name 名称
        
    .. py:method:: getParam(name)

        获取指定的参数
        
        .. note::

            所有的资金管理策略具有参数“auto-checkin”（bool类型，默认为False），其含义为“当账户现金不足以买入资金管理策略指示的买入数量时，自动向账户中补充存入（checkin）足够的现金。
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: setParam(name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: setTM(tm)
    
        :param TradeManager tm: 设置交易管理对象

    .. py:method:: setQuery(query)
    
        设置查询条件
    
        :param KQuery query:  查询条件
        
    .. py:method:: getQuery()
    
        获取查询条件
        
        :return: 查询条件
        :rtype: KQuery 
        
    .. py:method:: reset()
    
        复位操作
    
    .. py:method:: clone()
    
        克隆操作
        
    .. py:method:: getBuyNumber(datetime, stock, price, risk)
    
        获取指定交易对象可买入的数量
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
        :return: 可买入数量
        :rtype: int
        
    .. py:method:: getSellNumber(datetime, stock, price, risk)
    
        获取指定交易对象可卖出的数量
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
        :return: 可卖出数量
        :rtype: int
        
    .. py:method:: buyNotify(trade_record)
    
        【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载
        
        :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录
        
    .. py:method:: sellNotify(trade_record)
    
        【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载
        
        :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录
    
    .. py:method:: _getBuyNumber(datetime, stock, price, risk)

        【重载接口】获取指定交易对象可买入的数量
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
        :return: 可买入数量
        :rtype: int

    .. py:method:: _getSellNumber(datetime, stock, price, risk)
    
        【重载接口】获取指定交易对象可卖出的数量。如未重载，默认为卖出全部已持仓数量。
        
        :param Datetime datetime: 交易时间
        :param Stock stock: 交易对象
        :param float price: 交易价格
        :param float risk: 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
        :return: 可卖出数量
        :rtype: int
        
    .. py:method:: _reset()
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone()
    
        【重载接口】子类克隆接口