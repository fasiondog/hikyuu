.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

绩效统计
=========

.. py:class:: Performance

    简单绩效统计
    
    .. py:method:: reset(self)
    
        复位，清除已计算的结果
        
    .. py:method:: report(self, tm[, datetime=Datetime.now()])
    
        简单的文本统计报告，用于直接输出打印
        
        :param TradeManager tm: 指定的交易管理实例
        :param Datetime datetime: 统计截止时刻
        :rtype: str
    
    .. py:method:: statistics(self, tm[, datetime=Datetime.now()])
    
        根据交易记录，统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime

        :param TradeManager tm: 指定的交易管理实例
        :param Datetime datetime: 统计截止时刻      
    
    .. py:method:: get(self, name)
    
        按指标名称获取指标值，必须在运行 statistics 或 report 之后生效
        
        :param str name: 指标名称
        :rtype: float
    
    .. py:method:: __getitem__(self, name)
    
        同 get 方法。按指标名称获取指标值，必须在运行 statistics 或 report 之后生效
        
        :param str name: 指标名称
        :rtype: float

    