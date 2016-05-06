.. py:currentmodule:: hikyuu.trade_sys
.. highlightlang:: python

移滑价差算法
============

内建移滑价差算法
----------------

.. py:function:: SL_FixedPercent([p = 0.001])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)
    
    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例


自定义移滑价差算法
------------------

自定义移滑价差接口：

* :py:meth:`SlippageBase.getRealBuyPrice` - 【必须】计算实际买入价格
* :py:meth:`SlippageBase.getRealSellPrice` - 【必须】计算实际卖出价格
* :py:meth:`SlippageBase._calculate` - 【必须】子类计算接口
* :py:meth:`SlippageBase._clone` - 【必须】克隆接口
* :py:meth:`SlippageBase._reset` - 【可选】重载私有变量


移滑价差算法基类
----------------

.. py:class:: SlippageBase([name])

    移滑价差算法基类
    
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
        
    .. py:method:: setTO(k)
    
        :param KData k: 设置交易对象
        
    .. py:method:: getTO()
    
        :return: 交易对象
        :rtype: KData
        
    .. py:method:: getRealBuyPrice(datetime, price)

        【重载接口】计算实际买入价格
        
        :param Datetime datetime: 买入时间
        :param float price: 计划买入价格
        :return: 实际买入价格
        :rtype: float
        
    .. py:method:: getRealSellPrice(datetime, price)

        【重载接口】计算实际卖出价格
        
        :param Datetime datetime: 卖出时间
        :param float price: 计划卖出价格
        :return: 实际卖出价格
        :rtype: float        

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