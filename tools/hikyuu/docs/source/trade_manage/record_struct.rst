.. currentmodule:: hikyuu.trade_manage
.. highlightlang:: python

基础数据结构
============

交易业务类型
------------

.. py:function:: getBusinessName(business)

    :param BUSINESS business: 交易业务类型
    :return: 交易业务类型名称("INIT"|"BUY"|"SELL"|"GIFT"|"BONUS"|"CHECKIN"|"CHECKOUT"|"UNKNOWN"
    :rtype: string

.. py:data:: BUSINESS.INIT 

    建立初始账户
    
.. py:data:: BUSINESS.BUY

    买入
    
.. py:data:: BUSINESS.SELL

    卖出

.. py:data:: BUSINESS.GIFT

    送股

.. py:data:: BUSINESS.BONUS

    分红

.. py:data:: BUSINESS.CHECKIN

    存入现金
    
.. py:data:: BUSINESS.CHECKOUT

    取出现金
    
.. py:data:: BUSINESS.INVALID

    无效类型




交易成本记录
------------

交易成本计算的返回结果

.. py:class:: CostRecord

    交易成本记录

    .. py:attribute:: commission  佣金(double)
    .. py:attribute:: stamptax    印花税(double)
    .. py:attribute:: transferfee 过户费(double)
    .. py:attribute:: others      其它费用(double)
    .. py:attribute:: total       总成本(double)，= 佣金 + 印花税 + 过户费 + 其它费用
        
        
交易记录
--------

.. py:class:: TradeRecordList

    交易记录列表
    
    .. py:method:: to_np()
    
        仅在安装了numpy模块时生效，转换为numpy.array
    
    .. py:method:: to_df()
    
        仅在安装了pandas模块时生效，转换为pandas.DataFrame

.. py:class:: TradeRecord([stock, datetime, business, planPrice, realPrice, goalPrice, number, cost, stoploss, cash, part])

    交易记录
    
    .. py:attribute:: stock     股票（Stock）
    .. py:attribute:: datetime  交易时间（Datetime）
    .. py:attribute:: business  交易类型
    .. py:attribute:: planPrice 计划交易价格（double）
    .. py:attribute:: realPrice 实际交易价格（double）
    .. py:attribute:: goalPrice 目标价格（double），如果为0表示未限定目标
    .. py:attribute:: number    成交数量（int）
    .. py:attribute:: cost      交易成本

        类型：:py:class:`CostRecord`
        
    .. py:attribute:: stoploss 止损价（double）
    .. py:attribute:: cash     现金余额（double）
    .. py:attribute:: part     交易指示来源，区别是哪个部件发出的指示，constant.null_int表示无效

    

持仓记录
--------

.. py:class:: PositionRecordList

    持仓记录列表
    
    .. py:method:: to_np()
    
        仅在安装了numpy模块时生效，转换为numpy.array
    
    .. py:method:: to_df()
    
        仅在安装了pandas模块时生效，转换为pandas.DataFrame
        

.. py:class:: PositionRecord([stock, takeDatetime, cleanDatetime, number, stoploss, goalPrice, totalNumber, buyMoney, totalCost, totalRisk, sellMoney])

    持仓记录
    
    .. py:attribute:: stock         交易对象
    .. py:attribute:: takeDatetime  初次建仓日期
    .. py:attribute:: cleanDatetime 平仓日期，当前持仓记录中为Null<Datetime>()
    .. py:attribute:: number      当前持仓数量
    .. py:attribute:: stoploss    当前止损价
    .. py:attribute:: goalPrice   当前的目标价格
    .. py:attribute:: totalNumber 累计持仓数量
    .. py:attribute:: buyMoney    累计买入资金
    .. py:attribute:: totalCost   累计交易总成本
    .. py:attribute:: totalRisk   累计交易风险 = 各次 （买入价格-止损)*买入数量, 不包含交易成本
    .. py:attribute:: sellMoney   累计卖出资金

    
资产情况记录
------------

由TradeManager::getFunds返回

.. py:class:: FundsRecord([cash, market_value, short_market_value, base_cash, base_asset, borrow_cash, borrow_asset])

    当前资产情况记录，由TradeManager::getFunds返回
    
    .. py:attribute:: cash               当前现金（double）
    .. py:attribute:: market_value       当前多头市值（double）
    .. py:attribute:: short_market_value 当前空头仓位市值 （double）
    .. py:attribute:: base_cash          当前投入本金（double）
    .. py:attribute:: base_asset         当前投入的资产价值（double）
    .. py:attribute:: borrow_cash        当前借入的资金（double），即负债
    .. py:attribute:: borrow_asset       当前借入证券资产价值（double）