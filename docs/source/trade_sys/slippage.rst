.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

移滑价差算法|SP
===============

内建移滑价差算法
----------------

.. py:function:: SP_FixedPercent([p=0.001])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)
    
    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例


.. py:function:: SP_FixedValuet([p=0.001])

    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格
    
    :param float p: 偏移价格
    :return: 移滑价差算法实例

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05])

    均匀分布随机价格移滑价差算法, 买入和卖出操作是价格在[min_value, max_value]范围内的均匀分布随机偏移
    
    :param float min_value: 最小偏移价格
    :param float max_value: 最大偏移价格
    :return: 移滑价差算法实例

.. py:function:: SP_Normal([mean=0.0, stddev=0.05])

    正态分布随机价格移滑价差算法, 买入和卖出操作是价格在正态分布[mean, stddev]范围内的随机偏移
    
    :param float mean: 正态分布的均值
    :param float stddev: 正态分布的标准差
    :return: 移滑价差算法实例

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05])

    对数正态分布随机价格移滑价差算法, 买入和卖出操作是价格在对数正态分布[mean, stddev]范围内的随机偏移

    :param float mean: 对数正态分布的均值
    :param float stddev: 对数正态分布的标准差
    :return: 移滑价差算法实例

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])
         
    截断正态分布随机价格移滑价差算法, 买入和卖出操作是价格在截断正态分布[mean, stddev, min_value, max_value]范围内的随机偏移
    
    :param float mean: 截断正态分布的均值
    :param float stddev: 截断正态分布的标准差
    :param float min_value: 最小截断值
    :param float max_value: 最大截断值
    :return: 移滑价差算法实例
    

自定义移滑价差算法
------------------

自定义移滑价差接口：

* :py:meth:`SlippageBase.get_real_buy_price` - 【必须】计算实际买入价格
* :py:meth:`SlippageBase.get_real_sell_price` - 【必须】计算实际卖出价格
* :py:meth:`SlippageBase._calculate` - 【必须】子类计算接口
* :py:meth:`SlippageBase._clone` - 【必须】克隆接口
* :py:meth:`SlippageBase._reset` - 【可选】重载私有变量


移滑价差算法基类
----------------

.. py:class:: SlippageBase

    移滑价差算法基类
    
    .. py:attribute:: name 名称
    
    .. py:method:: __init__(self[, name="SlippageBase"])
    
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
        
    .. py:method:: get_real_buy_price(self, datetime, price)

        【重载接口】计算实际买入价格
        
        :param Datetime datetime: 买入时间
        :param float price: 计划买入价格
        :return: 实际买入价格
        :rtype: float
        
    .. py:method:: get_real_sell_price(self, datetime, price)

        【重载接口】计算实际卖出价格
        
        :param Datetime datetime: 卖出时间
        :param float price: 计划卖出价格
        :return: 实际卖出价格
        :rtype: float        

    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量
    
    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口          