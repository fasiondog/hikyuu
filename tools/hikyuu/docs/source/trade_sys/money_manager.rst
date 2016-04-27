.. py:currentmodule:: trade_sys
.. highlightlang:: python

资金管理策略
============

资金管理策略基类
----------------

.. py:class:: MoneyManagerBase([name])

    资金管理策略基类
    
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

    .. py:method:: setTM(tm)
    
        :param TradeManager tm: 设置交易管理对象

    .. py:method:: setKType(ktype)
    
        设置交易的K线类型
    
        :param KQuery.KType ktype: 