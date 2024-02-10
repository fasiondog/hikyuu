.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

市场环境判定策略
================

内建市场环境判定策略
--------------------

.. py:function:: EV_TwoLine(fast, slow[, market = 'SH'])

    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Indicator fast: 快线指标
    :param Indicator slow: 慢线指标
    :param string market: 市场名称

.. py:function:: EV_Bool(ind[, market = 'SH'])

    布尔信号指标市场环境

    :param Indicator ind: bool类型的指标, 指标中相应位置大于0则代表市场有效, 否则无效
    :param str market: 指定的市场，用于获取相应的交易日历


自定义市场环境判定策略
----------------------

自定义市场环境判定策略接口：

* :py:meth:`EnvironmentBase._calculate` - 【必须】子类计算接口
* :py:meth:`EnvironmentBase._clone` - 【必须】克隆接口
* :py:meth:`EnvironmentBase._reset` - 【可选】重载私有变量

市场环境判定策略基类
--------------------

.. py:class:: EnvironmentBase

    市场环境判定策略基类
    
    .. py:attribute:: name 名称
    .. py:attribute:: query 设置或获取查询条件
    
    .. py:method:: __init__(self[, name='EnvironmentBase'])
    
        初始化构造函数
        
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
        
    .. py:method:: is_valid(self, datetime)
    
        指定时间系统是否有效
        
        :param Datetime datetime: 指定时间
        :return: True 有效 | False 无效
    
    .. py:method:: _add_valid(self, datetime)
    
        加入有效时间，在_calculate中调用
        
        :param Datetime datetime: 有效时间
      
    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，用于复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口