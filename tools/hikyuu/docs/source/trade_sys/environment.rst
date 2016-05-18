.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

市场环境判定策略
================

内建市场环境判定策略
--------------------

.. py:function:: EV_TwoLine(fast, slow[, market = 'SH'])

    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Operand fast: 快线指标
    :param Operand slow: 慢线指标
    :param string market: 市场名称


自定义市场环境判定策略
----------------------

自定义市场环境判定策略接口：

* :py:meth:`EnvironmentBase._calculate` - 【必须】子类计算接口
* :py:meth:`EnvironmentBase._clone` - 【必须】克隆接口
* :py:meth:`EnvironmentBase._reset` - 【可选】重载私有变量

市场环境判定策略基类
--------------------

.. py:class:: EnvironmentBase([name])

    市场环境判定策略基类
    
    .. py:attribute:: name 名称
        
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
        
    .. py:method:: setQuery(query)
    
        设置查询条件
    
        :param KQuery query:  查询条件
        
    .. py:method:: getQuery()
    
        获取查询条件
        
        :return: 查询条件
        :rtype: KQuery 

    .. py:method:: isValid(datetime)
    
        指定时间系统是否有效
        
        :param Datetime datetime: 指定时间
        :return: True 有效 | False 无效
    
    .. py:method:: _addValid(datetime)
    
        加入有效时间，在_calculate中调用
        
        :param Datetime datetime: 有效时间
      
    .. py:method:: reset()
    
        复位操作
    
    .. py:method:: clone()
    
        克隆操作        
        
    .. py:method:: _calculate()
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset()
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone()
    
        【重载接口】子类克隆接口