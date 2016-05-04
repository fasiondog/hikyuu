.. currentmodule:: hikyuu.trade_manage
.. highlightlang:: python

交易管理类
==========

.. py:function:: crtTM([datetime = Datetime(199001010000), initcash = 100000, costfunc = TC_Zero(), name = "SYS"])

    :param Datetime datetime:
    :param double initcash:
    :param TradeCost costfunc:
    :param string name:
    :rtype: TradeManager
    
    
.. py:class:: TradeManager(datetime, initcash, costfunc, name)

    .. py:attribute:: name
        
        名称
        
    .. py:attribute:: costFunc 
        
        交易成本算法
        
    .. py:attribute:: initCash
        
        （只读）初始资金
        
    .. py:attribute:: initDatetime
        
        （只读）账户建立日期
        
    .. py:attribute:: firstDatetime 
        
        （只读）第一笔买入交易发生日期，如未发生交易返回Null<Datetime>()
        
    .. py:attribute:: lastDatetime
        
        （只读）最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期
        
    .. py:attribute:: reinvest 
        
        （只读）红利/股息/送股再投资标志
        
    .. py:attribute:: precision 
        
        （只读）
